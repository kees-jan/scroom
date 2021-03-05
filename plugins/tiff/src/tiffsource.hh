/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>
#include <set>
#include <string>

#include <tiffio.h>

#include <scroom/colormappable.hh>
#include <scroom/observable.hh>
#include <scroom/opentiledbitmapinterface.hh>
#include <scroom/pipetteviewinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/scroominterface.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/transformpresentation.hh>

namespace Scroom
{
  namespace Tiff
  {
    using TIFFPtr = boost::shared_ptr<TIFF>;

    using namespace Scroom::TiledBitmap;

    boost::optional<std::tuple<Scroom::TiledBitmap::BitmapMetaData, TIFFPtr>> open(const std::string& fileName);

    class Source : public SourcePresentation
    {
    private:
      std::string    fileName;
      TIFFPtr        preOpenedTif;
      TIFFPtr        tif;
      BitmapMetaData bmd;

    public:
      using Ptr = boost::shared_ptr<Source>;

      static Ptr create(std::string fileName, TIFFPtr tif, BitmapMetaData bmd);

      bool reset();

      // SourcePresenentation
      void        fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) override;
      void        done() override;
      std::string getName() override { return fileName; }

    private:
      Source(std::string fileName, TIFFPtr tif, BitmapMetaData bmd);
    };
  } // namespace Tiff
} // namespace Scroom
