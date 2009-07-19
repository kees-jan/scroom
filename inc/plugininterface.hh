#ifndef _RC/SCROOM.GIT/INC/PLUGININTERFACE.H
#define _RC/SCROOM.GIT/INC/PLUGININTERFACE.H

namespace Scroom
{
  class PluginInterface
  {
  }
}

extern "C"
{
  PluginInterface* getPluginInterface();
}




#endif
