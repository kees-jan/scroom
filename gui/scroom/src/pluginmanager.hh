#ifndef _PLUGINMANAGER_HH
#define _PLUGINMANAGER_HH

#include <gtk/gtk.h>

#include <list>
#include <string>

#include <workinterface.hh>

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

private:
  void setStatusBarMessage(const char* message);
  
public:

  PluginManager();
  
  virtual bool doWork();

  void addHook(GtkWidget* scroom);
};

void startPluginManager(GtkWidget* scroom);

#endif
