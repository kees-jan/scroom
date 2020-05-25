/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>

#include <scroom/scroomplugin.hh>
#include <scroom/unused.hh>

#include "measure.hh"

G_MODULE_EXPORT const gchar* g_module_check_init(GModule *module)
{
  UNUSED(module);
  printf("Measure plugin check_init function\n");
  return NULL; // success
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
  UNUSED(module);
  printf("Measure plugin unload function\n");
}

G_MODULE_EXPORT PluginInformationInterface::Ptr getPluginInformation()
{
  return Measure::create();
}
