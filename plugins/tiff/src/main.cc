/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2013 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <stdio.h>

#include <scroom/scroomplugin.hh>
#include <scroom/unused.h>

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


G_MODULE_EXPORT PluginInformationInterface::Ptr getPluginInformation()
{
  return Tiff::create();
}
