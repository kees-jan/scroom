/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "roiplugin.hh"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <scroom/roi.hh>

#include "roiui.hh"

RoiPlugin::RoiPlugin()
{
}

RoiPlugin::Ptr RoiPlugin::create()
{
  return Ptr(new RoiPlugin());
}

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

std::string RoiPlugin::getPluginName()
{
  return "RoiPlugin";
}

std::string RoiPlugin::getPluginVersion()
{
  return "0.0";
}

void RoiPlugin::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerOpenInterface("Roi viewer", shared_from_this<RoiPlugin>());
}

////////////////////////////////////////////////////////////////////////
// OpenInterface
////////////////////////////////////////////////////////////////////////

std::list<GtkFileFilter*> RoiPlugin::getFilters()
{
  std::list<GtkFileFilter*> result;

  GtkFileFilter* filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "Scroom Roi files");
  gtk_file_filter_add_pattern(filter, "*.sroi");
  gtk_file_filter_add_pattern(filter, "*.SROI");
  gtk_file_filter_add_mime_type(filter, "ascii/sroi");
  result.push_back(filter);
  
  return result;
}
  
void RoiPlugin::open(const std::string& fileName, ScroomInterface::Ptr const& scroomInterface)
{
  RoiUi::create(fileName, scroomInterface);
}
