#include <stdio.h>

#include <scroom/scroomplugin.hh>
#include <scroom/unused.hh>

#include "pipette.hh"

G_MODULE_EXPORT const gchar* g_module_check_init(GModule *module)
{
  UNUSED(module);
  printf("Pipette plugin check_init function\n");
  return NULL; // success
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
  UNUSED(module);
  printf("Pipette plugin unload function\n");
}

G_MODULE_EXPORT PluginInformationInterface::Ptr getPluginInformation()
{
  return Pipette::create();
}
