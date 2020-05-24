/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "pluginmanager.hh"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>

#include <scroom/plugininformationinterface.hh>
#include <boost/filesystem.hpp>

#include "callbacks.hh"

#ifdef _WIN32
#include <boost/dll.hpp>
#endif

const std::string SCROOM_PLUGIN_DIRS = "SCROOM_PLUGIN_DIRS";

static PluginManager::Ptr pluginManager = PluginManager::create();

PluginManager::PluginManager()
: devMode(false), state(FINDING_DIRECTORIES)
{
}

PluginManager::Ptr PluginManager::create()
{
  return Ptr(new PluginManager());
}

void startPluginManager(bool devMode)
{
  pluginManager->addHook(devMode);
}

bool PluginManager::doWork() 
{
  bool retval = true;

  gdk_threads_enter();
  switch (state)
  {
  case FINDING_DIRECTORIES: {
    setStatusBarMessage("Locating plugin directories");
    char* path = getenv(SCROOM_PLUGIN_DIRS.c_str());
    dirs.clear();

    if (!devMode) {
      #ifdef _WIN32
        // We want to keep everything portable on windows so we look for the plugin folder in the same directory as the .exe
        std::string plugin_path = (boost::dll::program_location().parent_path() / "plugins").generic_string();
        dirs.push_back(plugin_path);
      #else
        dirs.push_back(PLUGIN_DIR);
      #endif
    }

    if (path != nullptr) {
      printf("%s = %s\n", SCROOM_PLUGIN_DIRS.c_str(), path);

      for (char* i = path; *i != '\0'; i++) {

    	  // Windows uses semicolons for delimiting environment variables, Linux uses colons
		    #ifdef _WIN32
    	    const char envDelim = ';';
		    #else
    	    const char envDelim = ':';
		    #endif
          if (*i != envDelim) {
            continue;
          }

        *i = '\0';
        dirs.push_back(path);
        path = i + 1;
      }

      dirs.push_back(path);
    }

    currentDir = dirs.begin();
    files.clear();
    state = SCANNING_DIRECTORIES;
    break;
  }

  case SCANNING_DIRECTORIES: {
    setStatusBarMessage("Scanning plugin directories");
    if (currentDir == dirs.end()) {
      state = LOADING_FILES;
      currentFile = files.begin();
      break;
    }

    printf("Scanning directory: %s\n", currentDir->c_str());
    const char * folder = currentDir->c_str();
    namespace fs = boost::filesystem;
    boost::system::error_code ec;

    if (!fs::is_directory(folder, ec)) {
      printf("Can't open directory...\n");
      currentDir++;
      break;
    }

    for (const auto & entry : fs::directory_iterator(folder)) {
      if (fs::is_regular_file(entry) || fs::is_symlink(entry) || fs::is_other(entry)) {
        files.push_back(entry.path().generic_string());
      }
    }

    currentDir++;
    break;
  }

  case LOADING_FILES: {
    setStatusBarMessage("Loading Plugins");
    if (currentFile == files.end()) {
      state = DONE;
      break;
    }

#ifdef _WIN32
    // Only read .dll files
    if (currentFile->compare(currentFile->size() - 4, 4, ".dll") == 0) {
#else
    // Only read .so files
    if (currentFile->compare(currentFile->size() - 3, 3, ".so") == 0) {
#endif
        printf("Reading file: %s\n", currentFile->c_str());
        GModule* plugin = g_module_open(currentFile->c_str(), static_cast<GModuleFlags>(0));
        if (plugin) {
          // Need to pass a gpointer to g_module_symbol. If I pass a
          // PluginFunc, glib 2.16.6/gcc 4.2.4 will complain about
          // type-punned pointers.
          gpointer pgpi;
          if (g_module_symbol(plugin, "getPluginInformation", &pgpi)) {
            PluginFunc gpi = reinterpret_cast<PluginFunc>(pgpi);
            if (gpi) {
              PluginInformationInterface::Ptr pi = (*gpi)();
              if (pi) {
                if (pi->pluginApiVersion == PLUGIN_API_VERSION) {
                  pluginInformationList.push_back(PluginInformation(plugin, pi));
                  pi->registerCapabilities(shared_from_this<PluginManager>());
                  plugin = nullptr;
                  gpi = nullptr;
                  pi.reset();
                } else {
                  printf("Plugin has incorrect API version %d, instead of %d\n",
                         pi->pluginApiVersion, PLUGIN_API_VERSION);
                }
              } else {
                printf("GetPluginInformation returned NULL\n");
              }
            } else {
              printf("Can't find the getPluginInterface function: %s\n", g_module_error());
            }
          } else {
            printf("Can't lookup symbols: %s\n", g_module_error());
          }

          if (plugin) {
            g_module_close(plugin);
          }
        } else {
          printf("Something went wrong: %s\n", g_module_error());
        }
      }
    }
    currentFile++;
    break;
  case DONE: {
    setStatusBarMessage("Done loading plugins");
    on_done_loading_plugins();
    retval = false;
    break;
  }
  }
  gdk_threads_leave();
  return retval;
}

void PluginManager::setStatusBarMessage(const char*)
{
  // gtk_statusbar_pop(statusbar, status_context_id);
  // gtk_statusbar_push(statusbar, status_context_id, message);
  // printf("Statusbar update: %s\n", message);
}

void PluginManager::addHook(bool devMode_)
{
  this->devMode = devMode_;
  gtk_idle_add(on_idle, static_cast<WorkInterface*>(this));
  // progressbar = GTK_PROGRESS_BAR(lookup_widget(scroom, "progressbar"));
  // statusbar = GTK_STATUSBAR(lookup_widget(scroom, "statusbar"));
  //
  // status_context_id = gtk_statusbar_get_context_id(statusbar, "Plugin Manager");
  state = FINDING_DIRECTORIES;
}

void PluginManager::registerNewPresentationInterface(const std::string& identifier, NewPresentationInterface::Ptr newPresentationInterface)
{
  printf("I learned how to create a new %s!\n", identifier.c_str());
  newPresentationInterfaces[newPresentationInterface] = identifier;

  on_newPresentationInterfaces_update(newPresentationInterfaces);
}

void PluginManager::registerNewAggregateInterface(const std::string& identifier, NewAggregateInterface::Ptr newAggregateInterface)
{
  printf("I learned how to create a new %s aggregate!\n", identifier.c_str());
  newAggregateInterfaces[identifier] = newAggregateInterface;
}

void PluginManager::registerOpenPresentationInterface(const std::string& extension, OpenPresentationInterface::Ptr openPresentationInterface)
{
  printf("I learned how to open a %s file!\n", extension.c_str());

  openPresentationInterfaces[openPresentationInterface] = extension;
}

void PluginManager::registerOpenInterface(const std::string& extension, OpenInterface::Ptr openInterface)
{
  printf("I learned how to open a %s file!\n", extension.c_str());

  openInterfaces[openInterface] = extension;
}

void PluginManager::registerViewObserver(const std::string& identifier, ViewObserver::Ptr observer)
{
  printf("Observing Views for %s!\n", identifier.c_str());
  viewObservers[observer] = identifier;

  on_new_viewobserver(observer);
}

void PluginManager::registerPresentationObserver(const std::string& identifier, PresentationObserver::Ptr observer)
{
  printf("Observing Presentations for %s!\n", identifier.c_str());
  presentationObservers[observer] = identifier;
}

const std::map<NewPresentationInterface::Ptr, std::string>& PluginManager::getNewPresentationInterfaces()
{
  return newPresentationInterfaces;
}

const std::map<std::string, NewAggregateInterface::Ptr>& PluginManager::getNewAggregateInterfaces()
{
  return newAggregateInterfaces;
}

const std::map<OpenPresentationInterface::Ptr, std::string>& PluginManager::getOpenPresentationInterfaces()
{
  return openPresentationInterfaces;
}

const std::map<OpenInterface::Ptr, std::string>& PluginManager::getOpenInterfaces()
{
  return openInterfaces;
}

const std::map<ViewObserver::Ptr, std::string>& PluginManager::getViewObservers()
{
  return viewObservers;
}

const std::map<PresentationObserver::Ptr, std::string>& PluginManager::getPresentationObservers()
{
  return presentationObservers;
}

PluginManager::Ptr PluginManager::getInstance()
{
  return pluginManager;
}
