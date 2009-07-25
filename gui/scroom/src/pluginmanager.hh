#ifndef _PLUGINMANAGER_HH
#define _PLUGINMANAGER_HH

#include <gtk/gtk.h>

#include <list>
#include <string>
#include <map>

#include <scroomplugin.hh>
#include <scroominterface.hh>
#include <workinterface.hh>
#include <view.hh>

struct PluginInformation
{
  GModule* plugin;
  PluginInformationInterface* pluginInformation;

  PluginInformation(GModule* plugin, PluginInformationInterface* pluginInformation)
    : plugin(plugin), pluginInformation(pluginInformation)
  {
  }
};

class PluginManager : public WorkInterface, public ScroomInterface
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
  std::map<NewInterface*, std::string> newInterfaces;
  std::map<OpenInterface*, std::string> openInterfaces;
  std::list<View*> views;

private:
  void setStatusBarMessage(const char* message);
  
public:

  PluginManager();
  
  virtual bool doWork();

  void addHook(GtkWidget* scroom);

  virtual void registerNewInterface(const std::string& identifier, NewInterface* newInterface);
  virtual void unregisterNewInterface(NewInterface* newInterface);

  virtual void registerOpenInterface(const std::string& extension, OpenInterface* openInterface);
  virtual void unregisterOpenInterface(OpenInterface* openInterface);

  void registerView(View* view);
  void unregisterView(View* view);
};

void startPluginManager(GtkWidget* scroom);

#endif
