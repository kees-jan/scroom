#include <stdio.h>

#include <scroom/scroomplugin.hh>
#include <scroom/unused.hh>

#include "pipette.hh"

PluginInformationInterface::Ptr getPluginInformation()
{
  return Pipette::create();
}
