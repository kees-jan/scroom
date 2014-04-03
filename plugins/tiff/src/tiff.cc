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

#include "tiff.hh"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tiffpresentation.hh"

Tiff::Tiff()
{
}
Tiff::~Tiff()
{
}

Tiff::Ptr Tiff::create()
{
  return Ptr(new Tiff());
}

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

std::string Tiff::getPluginName()
{
  return "Tiff";
}

std::string Tiff::getPluginVersion()
{
  return "0.0";
}

void Tiff::registerCapabilities(ScroomInterface::Ptr host)
{
  host->registerOpenInterface("Tiff viewer", shared_from_this<Tiff>());
}

////////////////////////////////////////////////////////////////////////
// OpenInterface
////////////////////////////////////////////////////////////////////////

std::list<GtkFileFilter*> Tiff::getFilters()
{
  std::list<GtkFileFilter*> result;

  GtkFileFilter* filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "Tiff files");
#if MUTRACX_HACKS
  gtk_file_filter_add_pattern(filter, "*.tif");
  gtk_file_filter_add_pattern(filter, "*.tiff");
  gtk_file_filter_add_pattern(filter, "*.TIF");
  gtk_file_filter_add_pattern(filter, "*.TIFF");
#else
  gtk_file_filter_add_mime_type(filter, "image/tiff");
#endif
  result.push_back(filter);
  
  return result;
}
  
PresentationInterface::Ptr Tiff::open(const std::string& fileName)
{
  TiffPresentation::Ptr p = TiffPresentation::Ptr(new TiffPresentation());
  if(!p->load(fileName))
  {
    p.reset();
  }
  return p;
}
