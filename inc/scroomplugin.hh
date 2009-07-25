#ifndef _SCROOMPLUGIN_H
#define _SCROOMPLUGIN_H

#include <gmodule.h>

class PluginInformationInterface;

extern "C"
{
  typedef PluginInformationInterface* (*PluginFunc)();

  G_MODULE_IMPORT PluginInformationInterface* getPluginInformation();
}

#endif
