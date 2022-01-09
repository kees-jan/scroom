/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "pluginmanager.hh"

#include <cstdio>
#include <cstdlib>
#include <string>

#include <spdlog/spdlog.h>
#include <sys/types.h>

#include <boost/filesystem.hpp>

#include <scroom/plugininformationinterface.hh>

#include "callbacks.hh"

#ifdef _WIN32
#  include <boost/dll.hpp>
#endif

const std::string SCROOM_PLUGIN_DIRS = "SCROOM_PLUGIN_DIRS";

static PluginManager::Ptr pluginManager = PluginManager::create();

PluginManager::Ptr PluginManager::create() { return Ptr(new PluginManager()); }

void startPluginManager(bool devMode) { pluginManager->addHook(devMode); }

bool PluginManager::doWork()
{
  bool retval = true;

  switch(state)
  {
  case FINDING_DIRECTORIES:
  {
    setStatusBarMessage("Locating plugin directories");
    char* path = getenv(SCROOM_PLUGIN_DIRS.c_str());
    dirs.clear();

    if(!devMode)
    {
#ifdef _WIN32
      // We want to keep everything portable on windows so we look for the plugin folder in the same directory as the .exe
      std::string plugin_path = (boost::dll::program_location().parent_path() / "plugins").generic_string();
      dirs.push_back(plugin_path);
#else
      dirs.emplace_back(PLUGIN_DIR);
#endif
    }

    if(path != nullptr)
    {
      spdlog::debug("{} = {}", SCROOM_PLUGIN_DIRS, path);

      for(char* i = path; *i != '\0'; i++)
      {

// Windows uses semicolons for delimiting environment variables, Linux uses colons
#ifdef _WIN32
        const char envDelim = ';';
#else
        const char envDelim = ':';
#endif
        if(*i != envDelim)
        {
          continue;
        }

        *i = '\0';
        dirs.emplace_back(path);
        path = i + 1;
      }

      dirs.emplace_back(path);
    }

    currentDir = dirs.begin();
    files.clear();
    state = SCANNING_DIRECTORIES;
    break;
  }

  case SCANNING_DIRECTORIES:
  {
    setStatusBarMessage("Scanning plugin directories");
    if(currentDir == dirs.end())
    {
      state       = LOADING_FILES;
      currentFile = files.begin();
      break;
    }

    spdlog::debug("Scanning directory: {}", *currentDir);
    const char* folder = currentDir->c_str();
    namespace fs       = boost::filesystem;
    boost::system::error_code ec;

    if(!fs::is_directory(folder, ec))
    {
      spdlog::error("Can't open directory: {}", folder);
      currentDir++;
      break;
    }

    for(const auto& entry: fs::directory_iterator(folder))
    {
      if(fs::is_regular_file(entry) || fs::is_symlink(entry) || fs::is_other(entry))
      {
        files.push_back(entry.path().generic_string());
      }
    }

    currentDir++;
    break;
  }

  case LOADING_FILES:
  {
    setStatusBarMessage("Loading Plugins");
    if(currentFile == files.end())
    {
      state = DONE;
      break;
    }

#ifdef _WIN32
    // Only read .dll files
    if(currentFile->compare(currentFile->size() - 4, 4, ".dll") == 0)
    {
#else
    // Only read .so files
    if(currentFile->compare(currentFile->size() - 3, 3, ".so") == 0)
    {
#endif
      spdlog::debug("Reading file: {}", *currentFile);
      GModule* plugin = g_module_open(currentFile->c_str(), static_cast<GModuleFlags>(0));
      if(plugin)
      {
        gpointer pgpi         = nullptr;
        bool     symbol_found = false;

        if(g_module_symbol(plugin, "_Z20getPluginInformationv", &pgpi))
        {
          symbol_found = true;
        }
        else if(g_module_symbol(plugin, "getPluginInformation", &pgpi))
        {
          symbol_found = true;
          spdlog::warn("Plugin {} uses C-style GetPluginInformation - You need to recompile it", *currentFile);
        }

        if(symbol_found)
        {
          using PluginFunc = boost::shared_ptr<PluginInformationInterface> (*)();

          auto gpi = reinterpret_cast<PluginFunc>(pgpi);
          if(gpi)
          {
            PluginInformationInterface::Ptr pi = (*gpi)();
            if(pi)
            {
              if(pi->pluginApiVersion == PLUGIN_API_VERSION)
              {
                pluginInformationList.emplace_back(plugin, pi);
                pi->registerCapabilities(shared_from_this<PluginManager>());
                plugin = nullptr;
                gpi    = nullptr;
                pi.reset();
              }
              else
              {
                spdlog::error("Plugin {} has incorrect API version {}, instead of {}",
                              *currentFile,
                              pi->pluginApiVersion,
                              PLUGIN_API_VERSION);
              }
            }
            else
            {
              spdlog::error("GetPluginInformation returned NULL for file {}", *currentFile);
            }
          }
          else
          {
            spdlog::error("Can't find the getPluginInterface function in file {}: {}", *currentFile, g_module_error());
          }
        }
        else
        {
          spdlog::warn("Can't lookup symbols in file {}: {}", *currentFile, g_module_error());
        }

        if(plugin)
        {
          g_module_close(plugin);
        }
      }
      else
      {
        spdlog::error("Something went wrong for file {}: {}", *currentFile, g_module_error());
      }
    }
    currentFile++;
    break;
  }
  case DONE:
  {
    setStatusBarMessage("Done loading plugins");

    std::vector<std::pair<std::string, std::string>> pluginInfo;
    pluginInfo.reserve(pluginInformationList.size());
    size_t maxPluginNameLength = 0;
    for(const auto& plugin: pluginInformationList)
    {
      pluginInfo.emplace_back(plugin.pluginInformation->getPluginName(), plugin.pluginInformation->getPluginVersion());
      maxPluginNameLength = std::max(maxPluginNameLength, plugin.pluginInformation->getPluginName().size());
    }
    std::sort(pluginInfo.begin(), pluginInfo.end());
    spdlog::info("Loaded plugins:");
    for(const auto& [name, version]: pluginInfo)
    {
      spdlog::info("  {:{}} : {}", name, maxPluginNameLength, version);
    }

    on_done_loading_plugins();
    retval = false;
    break;
  }
  }
  return retval;
}

void PluginManager::setStatusBarMessage(const char*)
{
  // gtk_statusbar_pop(statusbar, status_context_id);
  // gtk_statusbar_push(statusbar, status_context_id, message);
}

void PluginManager::addHook(bool devMode_)
{
  devMode = devMode_;
  gdk_threads_add_idle(on_idle, static_cast<WorkInterface*>(this));
  // progressbar = GTK_PROGRESS_BAR(lookup_widget(scroom, "progressbar"));
  // statusbar = GTK_STATUSBAR(lookup_widget(scroom, "statusbar"));
  //
  // status_context_id = gtk_statusbar_get_context_id(statusbar, "Plugin Manager");
  state = FINDING_DIRECTORIES;
}

void PluginManager::registerNewPresentationInterface(const std::string&            identifier,
                                                     NewPresentationInterface::Ptr newPresentationInterface)
{
  newPresentationInterfaces[newPresentationInterface] = identifier;

  on_newPresentationInterfaces_update(newPresentationInterfaces);
}

void PluginManager::registerNewAggregateInterface(const std::string& identifier, NewAggregateInterface::Ptr newAggregateInterface)
{
  newAggregateInterfaces[identifier] = newAggregateInterface;
}

void PluginManager::registerOpenPresentationInterface(const std::string&             extension,
                                                      OpenPresentationInterface::Ptr openPresentationInterface)
{
  openPresentationInterfaces[openPresentationInterface] = extension;
}

void PluginManager::registerOpenTiledBitmapInterface(
  const std::string&                                               extension,
  boost::shared_ptr<Scroom::TiledBitmap::OpenTiledBitmapInterface> openTiledBitmapInterface)
{
  openTiledBitmapInterfaces[openTiledBitmapInterface]                               = extension;
  openPresentationInterfaces[ToOpenPresentationInterface(openTiledBitmapInterface)] = extension;
}

void PluginManager::registerOpenInterface(const std::string& extension, OpenInterface::Ptr openInterface)
{
  openInterfaces[openInterface] = extension;
}

void PluginManager::registerViewObserver(const std::string& identifier, ViewObserver::Ptr observer)
{
  viewObservers[observer] = identifier;

  on_new_viewobserver(observer);
}

void PluginManager::registerPresentationObserver(const std::string& identifier, PresentationObserver::Ptr observer)
{
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

const std::map<Scroom::TiledBitmap::OpenTiledBitmapInterface::Ptr, std::string>& PluginManager::getOpenTiledBitmapInterfaces()
{
  return openTiledBitmapInterfaces;
}

const std::map<OpenInterface::Ptr, std::string>& PluginManager::getOpenInterfaces() { return openInterfaces; }

const std::map<ViewObserver::Ptr, std::string>& PluginManager::getViewObservers() { return viewObservers; }

const std::map<PresentationObserver::Ptr, std::string>& PluginManager::getPresentationObservers()
{
  return presentationObservers;
}

PluginManager::Ptr PluginManager::getInstance() { return pluginManager; }
