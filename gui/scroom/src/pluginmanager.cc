#include "pluginmanager.hh"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <string>

#include <callbacks.hh>

const std::string SCROOM_PLUGIN_DIRS = "SCROOM_PLUGIN_DIRS";

static PluginManager pluginManager;

PluginManager::PluginManager()
  : progressbar(NULL), statusbar(NULL)
{
}

void startPluginManager(GtkWidget* scroom)
{
  pluginManager.addHook(scroom);
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
          if(content->d_type==DT_REG || content->d_type==DT_UNKNOWN)
          {
            files.push_back(g_module_build_path(currentDir->c_str(), content->d_name));
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
      printf("Reading file: %s\n", currentFile->c_str());
      plugin = g_module_open(currentFile->c_str(), G_MODULE_BIND_LOCAL);
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
              pluginInformationList.push_back(PluginInformation(plugin, pi));
              plugin = NULL;
              gpi = NULL;
              pi=NULL;
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
      currentFile++;
    }
    else
    {
      state = DONE;
    }
    break;
  case DONE:
    setStatusBarMessage("Done loading plugins");
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

void PluginManager::addHook(GtkWidget* scroom)
{
  gtk_idle_add(on_idle, static_cast<WorkInterface*>(this));
  // progressbar = GTK_PROGRESS_BAR(lookup_widget(scroom, "progressbar"));
  // statusbar = GTK_STATUSBAR(lookup_widget(scroom, "statusbar"));
  // 
  // status_context_id = gtk_statusbar_get_context_id(statusbar, "Plugin Manager");
  state = FINDING_DIRECTORIES;
}

void PluginManager::registerNewInterface(const std::string& identifier, NewInterface* newInterface)
{
  newInterfaces[newInterface] = identifier;
}

void PluginManager::unregisterNewInterface(NewInterface* newInterface)
{
  newInterfaces.erase(newInterface);
}

void PluginManager::registerOpenInterface(const std::string& extension, OpenInterface* openInterface)
{
  openInterfaces[openInterface] = extension;
}

void PluginManager::unregisterOpenInterface(OpenInterface* openInterface)
{
  openInterfaces.erase(openInterface);
}

