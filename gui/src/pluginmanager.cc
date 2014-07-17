/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "pluginmanager.hh"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <string>

#include <scroom/plugininformationinterface.hh>

#include "callbacks.hh"

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
  char* path = NULL;
  char* i=NULL;
  DIR* dir;
  struct dirent* content;
  GModule* plugin;
  
  gdk_threads_enter();
  switch(state)
  {
  case FINDING_DIRECTORIES:
    setStatusBarMessage("Locating plugin directories");
    path = getenv(SCROOM_PLUGIN_DIRS.c_str());
    dirs.clear();
    if(!devMode)
      dirs.push_back(PLUGIN_DIR);
    if(path!=NULL)
    {
      printf("%s=%s\n", SCROOM_PLUGIN_DIRS.c_str(), path);
      for(i=path; *i!=0; i++)
      {
        if(*i==':')
        {
          *i=0;
          dirs.push_back(path);
          path=i+1;
        }
      }
      dirs.push_back(path);
    }
    currentDir = dirs.begin();
    files.clear();
    state = SCANNING_DIRECTORIES;
    break;
    
  case SCANNING_DIRECTORIES:
    setStatusBarMessage("Scanning plugin directories");
    if(currentDir!=dirs.end())
    {
      printf("Scanning directory: %s\n", currentDir->c_str());
      dir = opendir(currentDir->c_str());
      if(dir!=NULL)
      {
        while( (content=readdir(dir)))
        {
          if(content->d_type==DT_REG ||
             content->d_type==DT_LNK ||
             content->d_type==DT_UNKNOWN)
          {
            files.push_back(g_build_path(G_DIR_SEPARATOR_S, currentDir->c_str(), content->d_name, NULL));
          }
        }
        closedir(dir);
      }
      else
      {
        printf("Can't open directory...\n");
      }
      currentDir++;
    }
    else
    {
      state = LOADING_FILES;
      currentFile=files.begin();
    }
    break;
  case LOADING_FILES:
    setStatusBarMessage("Loading Plugins");
    if(currentFile!=files.end())
    {
      if(!currentFile->compare(currentFile->size()-3, 3, ".so"))
      {
        printf("Reading file: %s\n", currentFile->c_str());
        plugin = g_module_open(currentFile->c_str(), (GModuleFlags)0);
        if(plugin)
        {
          // Need to pass a gpointer to g_module_symbol. If I pass a
          // PluginFunc, glib 2.16.6/gcc 4.2.4 will complain about
          // type-punned pointers.
          gpointer pgpi;
          if(g_module_symbol(plugin, "getPluginInformation", &pgpi))
          {
            PluginFunc gpi = (PluginFunc)pgpi;
            if(gpi)
            {
              PluginInformationInterface::Ptr pi = (*gpi)();
              if(pi)
              {
                if(pi->pluginApiVersion == PLUGIN_API_VERSION)
                {
                  pluginInformationList.push_back(PluginInformation(plugin, pi));
                  pi->registerCapabilities(shared_from_this<PluginManager>());
                  plugin = NULL;
                  gpi = NULL;
                  pi.reset();
                }
                else
                {
                  printf("Plugin has incorrect API version %d, instead of %d\n",
                         pi->pluginApiVersion, PLUGIN_API_VERSION);
                }
              }
              else
              {
                printf("GetPluginInformation returned NULL\n");
              }
            }
            else
            {
              printf("Can't find the getPluginInterface function: %s\n", g_module_error());
            }
          }
          else
          {
            printf("Can't lookup symbols: %s\n", g_module_error());
          }

          if(plugin)
          {
            g_module_close(plugin);
          }
        }
        else
        {
          printf("Something went wrong: %s\n", g_module_error());
        }
      }
      else
      {
        // printf("Skipping file: %s (doesn't end with \".so\")\n", currentFile->c_str());
      }
      currentFile++;
    }
    else
    {
      state = DONE;
    }
    break;
  case DONE:
    setStatusBarMessage("Done loading plugins");
    on_done_loading_plugins();
    retval = false;
    break;
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

void PluginManager::addHook(bool devMode)
{
  this->devMode = devMode;
  gtk_idle_add(on_idle, static_cast<WorkInterface*>(this));
  // progressbar = GTK_PROGRESS_BAR(lookup_widget(scroom, "progressbar"));
  // statusbar = GTK_STATUSBAR(lookup_widget(scroom, "statusbar"));
  // 
  // status_context_id = gtk_statusbar_get_context_id(statusbar, "Plugin Manager");
  state = FINDING_DIRECTORIES;
}

void PluginManager::registerNewInterface(const std::string& identifier, NewInterface::Ptr newInterface)
{
  printf("I learned how to create a new %s!\n", identifier.c_str());
  newInterfaces[newInterface] = identifier;

  on_newInterfaces_update(newInterfaces);
}

void PluginManager::registerNewAggregateInterface(const std::string& identifier, NewAggreagateInterface::Ptr newAggregateInterface)
{
  printf("I learned how to create a new %s aggregate!\n", identifier.c_str());
  newAggregateInterfaces[identifier] = newAggregateInterface;
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
}

void PluginManager::registerPresentationObserver(const std::string& identifier, PresentationObserver::Ptr observer)
{
  printf("Observing Presentations for %s!\n", identifier.c_str());
  presentationObservers[observer] = identifier;
}

const std::map<NewInterface::Ptr, std::string>& PluginManager::getNewInterfaces()
{
  return newInterfaces;
}

const std::map<std::string, NewAggreagateInterface::Ptr>& PluginManager::getNewAggregateInterfaces()
{
  return newAggregateInterfaces;
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
