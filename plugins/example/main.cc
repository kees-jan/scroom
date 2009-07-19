
#include <stdio.h>

#include <scroomplugin.hh>

G_MODULE_EXPORT const gchar* g_module_check_init(GModule *module)
{
  printf("Example plugin check_init function\n");
  return NULL; // success
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
  printf("Example plugin unload function\n");
}


G_MODULE_EXPORT PluginInformationInterface* getPluginInformation()
{
  printf("Example plugin says \"Hi\"\n");
  return NULL;
}
