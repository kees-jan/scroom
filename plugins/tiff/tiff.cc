/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

#include "tiffpresentation.hh"

Tiff::~Tiff()
{
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

void Tiff::registerCapabilities(ScroomInterface* host)
{
  host->registerOpenInterface("Tiff viewer", this);
}

void Tiff::unregisterCapabilities(ScroomInterface* host)
{
  host->unregisterOpenInterface(this);
}

////////////////////////////////////////////////////////////////////////
// OpenInterface
////////////////////////////////////////////////////////////////////////

std::list<GtkFileFilter*> Tiff::getFilters()
{
  std::list<GtkFileFilter*> result;

  GtkFileFilter* filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "Tiff files");
  gtk_file_filter_add_mime_type(filter, "image/tiff");
  result.push_back(filter);
  
  return result;
}
  
PresentationInterface::Ptr Tiff::open(const std::string& fileName, FileOperationObserver* observer)
{
  TiffPresentation* p = new TiffPresentation();
  if(!p->load(fileName, observer))
  {
    delete p;
    p=NULL;
  }
  return PresentationInterface::Ptr(p);
}
