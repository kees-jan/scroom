#ifndef _PLUGINMANAGER_HH
#define _PLUGINMANAGER_HH

#include <gtk/gtk.h>

#include <list>
#include <string>

#include <scroomplugin.hh>

#include <workinterface.hh>

struct PluginInformation
{
  GModule* plugin;
  PluginInformationInterface* pluginInformation;

  PluginInformation(GModule* plugin, PluginInformationInterface* pluginInformation)
    : plugin(plugin), pluginInformation(pluginInformation)
  {
  }
};

class PluginManager : public WorkInterface
{
private:
  typedef enum
    {
      FINDING_DIRECTORIES,
      SCANNING_DIRECTORIES,
      LOADING_FILES,
      DONE
    } PluginManagerState;
  
private:
  GtkProgressBar* progressbar;
  GtkStatusbar* statusbar;
  guint status_context_id;
  PluginManagerState state;
  std::list<std::string> dirs;
  std::list<std::string>::iterator currentDir;
  std::list<std::string> files;
  std::list<std::string>::iterator currentFile;
  std::list<PluginInformation> pluginInformationList;

private:
  void setStatusBarMessage(const char* message);
  
public:

  PluginManager();
  
  virtual bool doWork();

  void addHook(GtkWidget* scroom);
};

void startPluginManager(GtkWidget* scroom);

#endif
