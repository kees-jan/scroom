#ifndef _PLUGININTERFACE_H
#define _PLUGININTERFACE_H

namespace Scroom
{
  class PluginInterface
  {
  };
}

extern "C"
{
  typedef Scroom::PluginInterface* (*PluginFunc)();
  
  Scroom::PluginInterface* getPluginInterface();
}




#endif
