#ifndef _PLUGINMANAGER_HH
#define _PLUGINMANAGER_HH

#include <workinterface.hh>

class PluginManager : public WorkInterface
{

public:
  virtual bool doWork();

  void addHook();
};

void startPluginManager();

#endif
