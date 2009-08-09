
#include <stdio.h>

#include <scroomplugin.hh>
#include <unused.h>

#include "tiff.hh"

G_MODULE_EXPORT const gchar* g_module_check_init(GModule *module)
{
  UNUSED(module);
  printf("Tiff plugin check_init function\n");
  return NULL; // success
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
  UNUSED(module);
  printf("Tiff plugin unload function\n");
}


G_MODULE_EXPORT PluginInformationInterface* getPluginInformation()
{
  printf("Tiff plugin says \"Hi\"\n");
  return new Tiff();
}
