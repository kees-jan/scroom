#include "pluginmanager.hh"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <string>

#include <plugininformationinterface.hh>

#include <callbacks.hh>

const std::string SCROOM_PLUGIN_DIRS = "SCROOM_PLUGIN_DIRS";

static PluginManager pluginManager;

PluginManager::PluginManager()
{
}

void startPluginManager(bool devMode)
{
  pluginManager.addHook(devMode);
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
          PluginFunc gpi;
          if(g_module_symbol(plugin, "getPluginInformation", (gpointer*)&gpi))
          {
            if(gpi)
            {
              PluginInformationInterface* pi = (*gpi)();
              if(pi)
              {
                printf("Got the PluginInterface!\n");
                if(pi->pluginApiVersion == PLUGIN_API_VERSION)
                {
                  printf("Requesting registration\n");
                  pluginInformationList.push_back(PluginInformation(plugin, pi));
                  pi->registerCapabilities(this);
                  plugin = NULL;
                  gpi = NULL;
                  pi=NULL;
                }
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

void PluginManager::setStatusBarMessage(const char* message)
{
  // gtk_statusbar_pop(statusbar, status_context_id);
  // gtk_statusbar_push(statusbar, status_context_id, message);
  printf("Statusbar update: %s\n", message);
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

void PluginManager::registerNewInterface(const std::string& identifier, NewInterface* newInterface)
{
  printf("I learned how to create a new %s!\n", identifier.c_str());
  newInterfaces[newInterface] = identifier;

  on_newInterfaces_update(newInterfaces);
}

void PluginManager::unregisterNewInterface(NewInterface* newInterface)
{
  printf("I just forgot how to create a new %s!\n", newInterfaces[newInterface].c_str());

  newInterfaces.erase(newInterface);

  on_newInterfaces_update(newInterfaces);
}

void PluginManager::registerOpenInterface(const std::string& extension, OpenInterface* openInterface)
{
  printf("I learned how to open a %s file!\n", extension.c_str());

  openInterfaces[openInterface] = extension;
}

void PluginManager::unregisterOpenInterface(OpenInterface* openInterface)
{
  printf("I just forgot how to open a %s file!\n", openInterfaces[openInterface].c_str());

  openInterfaces.erase(openInterface);
}

void PluginManager::registerViewObserver(const std::string& identifier, ViewObserver* observer)
{
  printf("Observing Views for %s!\n", identifier.c_str());
  viewObservers[observer] = identifier;
}

void PluginManager::unregisterViewObserver(ViewObserver* observer)
{
  printf("I stopped observing views for %s!\n", viewObservers[observer].c_str());
  viewObservers.erase(observer);
}

void PluginManager::registerPresentationObserver(const std::string& identifier, PresentationObserver* observer)
{
  printf("Observing Presentations for %s!\n", identifier.c_str());
  presentationObservers[observer] = identifier;
}

void PluginManager::unregisterPresentationObserver(PresentationObserver* observer)
{
  printf("I stopped observing presentations for %s!\n", presentationObservers[observer].c_str());
  presentationObservers.erase(observer);
}

const std::map<NewInterface*, std::string>& PluginManager::getNewInterfaces()
{
  return newInterfaces;
}

const std::map<OpenInterface*, std::string>& PluginManager::getOpenInterfaces()
{
  return openInterfaces;
}

const std::map<ViewObserver*, std::string>& PluginManager::getViewObservers()
{
  return viewObservers;
}

const std::map<PresentationObserver*, std::string>& PluginManager::getPresentationObservers()
{
  return presentationObservers;
}

PluginManager& PluginManager::getInstance()
{
  return pluginManager;
}
