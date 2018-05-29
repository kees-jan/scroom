/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
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

void Tiff::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerOpenPresentationInterface("Tiff viewer", shared_from_this<Tiff>());
}

////////////////////////////////////////////////////////////////////////
// OpenPresentationInterface
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

PresentationInterface::Ptr Tiff::open(const std::string& fileName)
{
  TiffPresentationWrapper::Ptr p = TiffPresentationWrapper::create();
  if(!p->load(fileName))
  {
    p.reset();
  }
  return p;
}
