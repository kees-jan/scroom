/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "tiffsource.hh"

#include <utility>

#include <spdlog/spdlog.h>

namespace
{
  using TagInfo = std::pair<ttag_t, std::string>;
  using namespace Scroom::TiledBitmap;
  using Scroom::Tiff::TIFFPtr;

  template <typename T>
  T TIFFGetFieldChecked(const TIFFPtr& file, const TagInfo& tag)
  {
    T result{};

    if(1 != TIFFGetField(file.get(), tag.first, &result))
    {
      throw std::invalid_argument("Field not present in tiff file: " + std::string(tag.second));
    }

    return result;
  }

  template <typename T>
  T TIFFGetFieldCheckedOr(const TIFFPtr& file, const TagInfo& tag, T default_value)
  {
    T result{};

    if(1 != TIFFGetField(file.get(), tag.first, &result))
    {
      return default_value;
    }

    return result;
  }

  bool approx(const BitmapMetaData& left, const BitmapMetaData& right)
  {
    return left.type == right.type && left.bitsPerSample == right.bitsPerSample && left.samplesPerPixel == right.samplesPerPixel
           && left.rect == right.rect && static_cast<bool>(left.colormapHelper) == static_cast<bool>(right.colormapHelper);
  }

  void TIFFCloseUnlessNull(TIFF* tif)
  {
    if(tif)
    {
      TIFFClose(tif);
    }
  }
} // namespace

#define TT(x) TagInfo((x), #x)

namespace Scroom::Tiff
{
  using namespace Scroom::TiledBitmap;

  Colormap::Ptr getColorMap(const TIFFPtr& tif, uint16_t bps)
  {
    uint16_t*     r{};
    uint16_t*     g{};
    uint16_t*     b{};
    Colormap::Ptr colormap;

    const int result = TIFFGetField(tif.get(), TIFFTAG_COLORMAP, &r, &g, &b);
    if(result == 1)
    {
      colormap           = Colormap::create();
      colormap->name     = "Original";
      const size_t count = 1UL << bps;
      colormap->colors.resize(count);

      for(size_t i = 0; i < count; i++)
      {
        colormap->colors[i] = Color(1.0 * r[i] / 0xFFFF, 1.0 * g[i] / 0xFFFF, 1.0 * b[i] / 0xFFFF);
      }
    }

    return colormap;
  }

  boost::optional<Scroom::Utils::Point<double>> getAspectRatio(const TIFFPtr& tif)
  {
    float    resolutionX{};
    float    resolutionY{};
    uint16_t resolutionUnit{};

    if(TIFFGetField(tif.get(), TIFFTAG_XRESOLUTION, &resolutionX) && TIFFGetField(tif.get(), TIFFTAG_YRESOLUTION, &resolutionY)
       && TIFFGetField(tif.get(), TIFFTAG_RESOLUTIONUNIT, &resolutionUnit))
    {
      if(resolutionUnit != RESUNIT_NONE)
      {
        // Fix aspect ratio only
        float const base = std::max(resolutionX, resolutionY);
        resolutionX      = base / resolutionX;
        resolutionY      = base / resolutionY;
      }
      return Scroom::Utils::make_point<double>(resolutionX, resolutionY);
    }

    return {};
  }

  ColormapHelperBase::Ptr getColormapHelper(const TIFFPtr& tif, uint16_t bps)
  {
    auto colorMap = getColorMap(tif, bps);
    return colorMap ? ColormapHelper::create(colorMap) : nullptr;
  }

  boost::optional<std::tuple<Scroom::TiledBitmap::BitmapMetaData, TIFFPtr>> open(const std::string& fileName)
  {
    try
    {
      TIFFPtr const tif(TIFFOpen(fileName.c_str(), "r"), &TIFFCloseUnlessNull);
      if(!tif)
      {
        // Todo: report error
        spdlog::error("Failed to open file {}", fileName);
        return {};
      }

      auto spp    = TIFFGetFieldCheckedOr<uint16_t>(tif, TT(TIFFTAG_SAMPLESPERPIXEL), 1); // Default value, according to tiff spec
      auto bps    = TIFFGetFieldCheckedOr<uint16_t>(tif, TT(TIFFTAG_BITSPERSAMPLE), (spp == 1) ? 1 : 8);
      auto width  = TIFFGetFieldChecked<uint32_t>(tif, TT(TIFFTAG_IMAGEWIDTH));
      auto height = TIFFGetFieldChecked<uint32_t>(tif, TT(TIFFTAG_IMAGELENGTH));
      auto photometric = TIFFGetFieldChecked<uint16_t>(tif, TT(TIFFTAG_PHOTOMETRIC));

      ColormapHelperBase::Ptr colormapHelper = getColormapHelper(tif, bps);

      if(photometric != PHOTOMETRIC_PALETTE && colormapHelper)
      {
        spdlog::warn("Tiff contains a colormap, but photometric isn't palette");
        colormapHelper.reset();
      }

      auto numberOfGreyscaleColors = (bps == 2 || bps == 4) ? (1 << bps) : 2; // Yuk
      switch(photometric)
      {
      case PHOTOMETRIC_MINISBLACK:
        colormapHelper = MonochromeColormapHelper::create(numberOfGreyscaleColors);
        break;

      case PHOTOMETRIC_MINISWHITE:
        colormapHelper = MonochromeColormapHelper::createInverted(numberOfGreyscaleColors);
        break;

      case PHOTOMETRIC_PALETTE:
        if(!colormapHelper)
        {
          spdlog::warn("Photometric is palette, but tiff doesn't contain a colormap");
          colormapHelper = ColormapHelper::create(1 << bps);
        }
        break;

      case PHOTOMETRIC_RGB:
      case PHOTOMETRIC_SEPARATED:
        break;

      default:
        spdlog::error("Unrecognized value {} for photometric", photometric);
        return {};
      }

      auto aspectRatio = getAspectRatio(tif);
      if(aspectRatio)
      {
        spdlog::debug("This bitmap has size {}*{}, aspect ratio {:.2}*{:.2}", width, height, aspectRatio->x, aspectRatio->y);
      }
      else
      {
        spdlog::debug("This bitmap has size {}*{}", width, height);
      }

      BitmapMetaData bmd{{}, bps, spp, Scroom::Utils::make_rect<int>(0, 0, width, height), aspectRatio, colormapHelper};

      if(bps != 1 && bps != 2 && bps != 4 && bps != 8)
      {
        spdlog::error("{} bits per sample not supported (yet)", bps);
        return {};
      }

      if(spp == 4)
      {
        bmd.type = CMYK;
      }
      else if(spp == 3)
      {
        bmd.type = RGB;

        if(bps != 8)
        {
          spdlog::error("A RGB bitmap with {} samples per pixel isn't supported (yet)", bps);
          return {};
        }
      }
      else if(spp == 1)
      {
        bmd.type = (photometric == PHOTOMETRIC_PALETTE) ? Colormapped : Greyscale;
      }
      else
      {
        spdlog::error("{} samples per pixel not supported (yet)", spp);
        return {};
      }

      return std::make_tuple(bmd, tif);
    }
    catch(const std::exception& ex)
    {
      spdlog::error("{}", ex.what());
      return {};
    }
  }

  Source::Ptr Source::create(std::string fileName, TIFFPtr tif, BitmapMetaData bmd)
  {
    return Ptr(new Source(std::move(fileName), std::move(tif), std::move(bmd)));
  }

  Source::Source(std::string fileName_, TIFFPtr tif_, BitmapMetaData bmd_)
    : fileName(std::move(fileName_))
    , preOpenedTif(std::move(tif_))
    , bmd(std::move(bmd_))
  {
  }

  bool Source::reset()
  {
    tif          = preOpenedTif;
    preOpenedTif = nullptr;

    if(!tif)
    {
      auto r = Scroom::Tiff::open(fileName);
      if(r && approx(std::get<0>(*r), bmd))
      {
        tif = std::get<1>(*r);
      }
      else
      {
        if(r)
        {
          spdlog::error("Can't reload. Bitmap changed too much");
          spdlog::info("Previously: {}", to_string(bmd));
          spdlog::info("Now:        {}", to_string(std::get<0>(*r)));
        }
        // if (!r) then the error has already been reported by open()
        return false;
      }
    }
    ensure(tif);

    return true;
  }

  void Source::fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles)
  {
    auto spp = bmd.samplesPerPixel;
    auto bps = bmd.bitsPerSample;

    const auto        startLine_   = static_cast<uint32_t>(startLine);
    const auto        firstTile_   = static_cast<size_t>(firstTile);
    const auto        scanLineSize = static_cast<size_t>(TIFFScanlineSize(tif.get()));
    const auto        tileStride   = static_cast<size_t>(tileWidth * spp * bps / 8);
    std::vector<byte> row(scanLineSize);

    const size_t tileCount = tiles.size();
    auto         dataPtr   = std::vector<byte*>(tileCount);
    for(size_t tile = 0; tile < tileCount; tile++)
    {
      dataPtr[tile] = tiles[tile]->data.get();
    }

    for(size_t i = 0; i < static_cast<size_t>(lineCount); i++)
    {
      TIFFReadScanline(tif.get(), row.data(), static_cast<uint32_t>(i) + startLine_);

      for(size_t tile = 0; tile < tileCount - 1; tile++)
      {
        memcpy(dataPtr[tile], row.data() + (firstTile_ + tile) * tileStride, tileStride);
        dataPtr[tile] += tileStride;
      }
      memcpy(dataPtr[tileCount - 1],
             row.data() + (firstTile_ + tileCount - 1) * tileStride,
             scanLineSize - (firstTile_ + tileCount - 1) * tileStride);
      dataPtr[tileCount - 1] += tileStride;
    }
  }

  void Source::done() { tif.reset(); }

} // namespace Scroom::Tiff
