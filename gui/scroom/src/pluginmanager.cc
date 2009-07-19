#include "pluginmanager.hh"

#include <stdio.h>

#include <callbacks.h>

static PluginManager pluginManager;

bool PluginManager::doWork()
{
  printf("Hello world...\n");
  return true;
}

void PluginManager::addHook()
{
  gtk_idle_add(on_idle, static_cast<WorkInterface*>(this));
}

void startPluginManager()
{
  pluginManager.addHook();
}
