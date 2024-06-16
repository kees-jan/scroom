/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "pnm.hh"

#include <scroom/opentiledbitmapinterface.hh>
#include <scroom/transformpresentation.hh>

#include "pnmsource.hh"
#include "version.h"

using namespace Scroom::TiledBitmap;
using namespace Scroom::Utils;

Pnm::Ptr Pnm::create() { return Ptr(new Pnm()); }

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

std::string Pnm::getPluginName() { return "Pnm"; }

std::string Pnm::getPluginVersion() { return PACKAGE_VERSION; }

void Pnm::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerOpenTiledBitmapInterface("Pnm viewer", shared_from_this<Pnm>());
}

////////////////////////////////////////////////////////////////////////
// OpenTiledBitmapInterface
////////////////////////////////////////////////////////////////////////

std::list<GtkFileFilter*> Pnm::getFilters()
{
  std::list<GtkFileFilter*> result;

  GtkFileFilter* filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "Pnm files");
  gtk_file_filter_add_pattern(filter, "*.pnm");
  gtk_file_filter_add_pattern(filter, "*.pgm");
  gtk_file_filter_add_pattern(filter, "*.ppm");
  gtk_file_filter_add_pattern(filter, "*.pbm");
  result.push_back(filter);

  return result;
}

std::tuple<BitmapMetaData, Layer::Ptr, ReloadFunction> Pnm::open(const std::string& fileName)
{
  auto r = Scroom::Pnm::open(m_logger, fileName);
  if(r)
  {
    auto [bitmapMetaData, pnm, sourceType] = std::move(*r);

    auto layer = Layer::create(
      bitmapMetaData.rect.getWidth(),
      bitmapMetaData.rect.getHeight(),
      bitmapMetaData.bitsPerSample * bitmapMetaData.samplesPerPixel
    );

    Scroom::Pnm::Source::Ptr sourcePresentation;
    if(sourceType == Scroom::Pnm::SourceType::Ascii)
    {
      sourcePresentation = Scroom::Pnm::AsciiSource::create(fileName, std::move(pnm), bitmapMetaData);
    }
    else if(sourceType == Scroom::Pnm::SourceType::Ascii1bpp)
    {
      sourcePresentation = Scroom::Pnm::AsciiSource1bpp::create(fileName, std::move(pnm), bitmapMetaData);
    }
    else
    {
      sourcePresentation = Scroom::Pnm::BinarySource::create(fileName, std::move(pnm), bitmapMetaData);
    }

    auto load = [sourcePresentation, layer](const ProgressInterface::Ptr& progressInterface)
    {
      return sourcePresentation->resetPresentation() ? scheduleLoadingBitmap(sourcePresentation, layer, progressInterface)
                                                     : Empty();
    };

    return {bitmapMetaData, layer, load};
  }
  return {{}, {}, {}};
}
