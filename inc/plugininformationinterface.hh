#ifndef _PLUGININFORMATIONINTERFACE_H
#define _PLUGININFORMATIONINTERFACE_H

#include <scroominterface.hh>

#define PLUGIN_API_VERSION 0

class PluginInformationInterface
{
public:
  const int pluginApiVersion;

public:
  PluginInformationInterface()
    : pluginApiVersion(PLUGIN_API_VERSION)
  {}

  virtual std::string getPluginName()=0;
  virtual std::string getPluginVersion()=0;
  virtual void registerCapabilities(ScroomInterface* host)=0;
  virtual void unregisterCapabilities(ScroomInterface* host)=0;
  
};



#endif
