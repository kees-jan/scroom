/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

#include "roiplugin.hh"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <scroom/roi.hh>

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
  Scroom::Roi::List list = Scroom::Roi::parse(fileName);
  std::set<ViewObservable::Ptr> views = list.instantiate(scroomInterface, fileName);
}
