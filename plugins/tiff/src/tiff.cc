/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "tiff.hh"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <tiffio.h>

#include <scroom/opentiledbitmapinterface.hh>
#include <scroom/transformpresentation.hh>

#include "tiffsource.hh"

using namespace Scroom::TiledBitmap;
using namespace Scroom::Utils;

Tiff::Ptr Tiff::create() { return Ptr(new Tiff()); }

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

std::string Tiff::getPluginName() { return "Tiff"; }

std::string Tiff::getPluginVersion() { return "0.0"; }

void Tiff::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerOpenTiledBitmapInterface("Tiff viewer", shared_from_this<Tiff>());
}

////////////////////////////////////////////////////////////////////////
// OpenTiledBitmapInterface
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

std::tuple<BitmapMetaData, Layer::Ptr, ReloadFunction> Tiff::open(const std::string& fileName)
{
  auto r = Scroom::Tiff::open(fileName);
  if(r)
  {
    auto bmd = std::get<0>(*r);
    auto tif = std::get<1>(*r);

    auto layer = Layer::create(bmd.rect.getWidth(), bmd.rect.getHeight(), bmd.bitsPerSample * bmd.samplesPerPixel);

    auto sp = Scroom::Tiff::Source::create(fileName, tif, bmd);

    auto load = [sp, layer](const ProgressInterface::Ptr& pi) {
      return sp->reset() ? scheduleLoadingBitmap(sp, layer, pi) : Empty();
    };

    return {bmd, layer, load};
  }
  return {{}, {}, {}};
}
