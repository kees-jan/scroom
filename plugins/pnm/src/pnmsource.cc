/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "pnmsource.hh"

#include <limits>
#include <utility>

namespace
{
  using Scroom::TiledBitmap::BitmapMetaData;

  ////////////////////////////////////////////////////////////////////////
  // Helpers
  ////////////////////////////////////////////////////////////////////////

  bool approx(const BitmapMetaData& left, const BitmapMetaData& right)
  {
    return left.type == right.type && left.bitsPerSample == right.bitsPerSample && left.samplesPerPixel == right.samplesPerPixel
           && left.rect == right.rect && static_cast<bool>(left.colormapHelper) == static_cast<bool>(right.colormapHelper);
  }

  void skipComments(std::istream& s)
  {
    while((s >> std::ws).peek() == '#')
    {
      s.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  }

  void consumeDataBoundary(std::istream& s)
  {
    // The last header token is followed by exactly one whitespace character.
    // That character may be the newline ending a comment line.
    if(s.get() == '#')
    {
      s.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  }
} // namespace

namespace Scroom::Pnm
{
  using namespace Scroom::TiledBitmap;

  ////////////////////////////////////////////////////////////////////////
  // open()
  ////////////////////////////////////////////////////////////////////////

  boost::optional<std::tuple<Scroom::TiledBitmap::BitmapMetaData, std::ifstream, SourceType>>
    open(const Scroom::Logger& logger, const std::string& fileName)
  {
    try
    {
      std::ifstream pnm(fileName);
      pnm.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
      std::string fileType;
      pnm >> fileType;
      skipComments(pnm);
      int width = 0;
      int height = 0;
      pnm >> width;
      skipComments(pnm);
      pnm >> height;
      if(width < 0)
      {
        throw std::invalid_argument("Width cannot be negative");
      }
      if(height < 0)
      {
        throw std::invalid_argument("Height cannot be negative");
      }

      logger->debug("This bitmap has size {}*{}", width, height);

      const auto rect = Scroom::Utils::make_rect<int>(0, 0, width, height);
      BitmapMetaData bmd{};
      SourceType sourceType = SourceType::Binary;
      if(fileType == "P1")
      {
        auto colormap = Colormap::create();
        colormap->name = "Default";
        colormap->colors = {Color(1.0, 1.0, 1.0), Color(0.0, 0.0, 0.0)};
        bmd = {Colormapped, 1, 1, rect, {}, ColormapHelper::create(colormap)};
        sourceType = SourceType::Ascii1bpp;
      }
      else if(fileType == "P4")
      {
        consumeDataBoundary(pnm);
        // Netpbm convention: 0=white, 1=black
        auto colormap = Colormap::create();
        colormap->name = "Default";
        colormap->colors = {Color(1.0, 1.0, 1.0), Color(0.0, 0.0, 0.0)};
        bmd = {Colormapped, 1, 1, rect, {}, ColormapHelper::create(colormap)};
      }
      else
      {
        skipComments(pnm);
        int maxVal = 0;
        pnm >> maxVal;
        if(maxVal != 255)
        {
          throw std::invalid_argument("Only 8bpp is supported");
        }

        if(fileType == "P2")
        {
          bmd = {Greyscale, 8, 1, rect, {}, MonochromeColormapHelper::create(2)};
          sourceType = SourceType::Ascii;
        }
        else if(fileType == "P3")
        {
          bmd = {RGB, 8, 3, rect, {}, nullptr};
          sourceType = SourceType::Ascii;
        }
        else
        {
          consumeDataBoundary(pnm);
          if(fileType == "P5")
          {
            bmd = {Greyscale, 8, 1, rect, {}, MonochromeColormapHelper::create(2)};
          }
          else if(fileType == "P6")
          {
            bmd = {RGB, 8, 3, rect, {}, nullptr};
          }
          else
          {
            throw std::invalid_argument("Unsupported file type: " + fileType);
          }
        }
      }
      return std::make_tuple(bmd, std::move(pnm), sourceType);
    }
    catch(const std::exception& ex)
    {
      logger->error("While processing file {}: {}", fileName, ex.what());
      return {};
    }
  }

  ////////////////////////////////////////////////////////////////////////
  // Source
  ////////////////////////////////////////////////////////////////////////

  Source::Source(std::string fileName, std::ifstream pnm, BitmapMetaData bmd)
    : m_fileName(std::move(fileName))
    , m_preOpenedPnm(std::move(pnm))
    , m_bmd(std::move(bmd))
  {
  }

  bool Source::resetPresentation()
  {
    m_pnm = std::move(m_preOpenedPnm);

    if(!m_pnm)
    {
      auto r = Scroom::Pnm::open(m_logger, m_fileName);
      if(r && approx(std::get<0>(*r), m_bmd))
      {
        m_pnm = std::move(std::get<1>(*r));
      }
      else
      {
        if(r)
        {
          m_logger->error("Can't reload. Bitmap changed too much");
          m_logger->info("Previously: {}", to_string(m_bmd));
          m_logger->info("Now:        {}", to_string(std::get<0>(*r)));
        }
        // if (!r) then the error has already been reported by open()
        return false;
      }
    }
    ensure(m_pnm);

    return true;
  }

  void Source::done() { m_pnm.close(); }

  ////////////////////////////////////////////////////////////////////////
  // BinarySource
  ////////////////////////////////////////////////////////////////////////

  BinarySource::Ptr BinarySource::create(std::string fileName, std::ifstream pnm, BitmapMetaData bmd)
  {
    return Ptr(new BinarySource(std::move(fileName), std::move(pnm), std::move(bmd)));
  }

  void BinarySource::fillTiles(int /*startLine*/, int lineCount, int tileWidth, int firstTile_, std::vector<Tile::Ptr>& tiles)
  {
    auto spp = m_bmd.samplesPerPixel;
    auto bps = m_bmd.bitsPerSample;

    const auto firstTile = static_cast<size_t>(firstTile_);
    const auto scanLineSize = static_cast<std::streamsize>((m_bmd.rect.width() * spp * bps + 7) / 8);
    const auto tileStride = static_cast<size_t>(tileWidth * spp * bps / 8);
    std::vector<char> row(scanLineSize);

    const size_t tileCount = tiles.size();
    auto dataPtr = std::vector<byte*>(tileCount);
    for(size_t tile = 0; tile < tileCount; tile++)
    {
      dataPtr[tile] = tiles[tile]->data.get();
    }

    for(size_t i = 0; i < static_cast<size_t>(lineCount); i++)
    {
      m_pnm.read(row.data(), scanLineSize);

      for(size_t tile = 0; tile < tileCount - 1; tile++)
      {
        memcpy(dataPtr[tile], row.data() + (firstTile + tile) * tileStride, tileStride);
        dataPtr[tile] += tileStride;
      }
      memcpy(
        dataPtr[tileCount - 1],
        row.data() + (firstTile + tileCount - 1) * tileStride,
        scanLineSize - (firstTile + tileCount - 1) * tileStride
      );
      dataPtr[tileCount - 1] += tileStride;
    }
  }

  ////////////////////////////////////////////////////////////////////////
  // AsciiSource
  ////////////////////////////////////////////////////////////////////////

  AsciiSource::Ptr AsciiSource::create(std::string fileName, std::ifstream pnm, BitmapMetaData bmd)
  {
    return Ptr(new AsciiSource(std::move(fileName), std::move(pnm), std::move(bmd)));
  }

  void AsciiSource::fillTiles(int /*startLine*/, int lineCount, int tileWidth, int firstTile_, std::vector<Tile::Ptr>& tiles)
  {
    auto spp = m_bmd.samplesPerPixel;
    auto bps = m_bmd.bitsPerSample;

    const auto firstTile = static_cast<size_t>(firstTile_);
    const auto rowSamples = static_cast<size_t>(m_bmd.rect.width() * spp * bps / 8);
    const auto tileStride = static_cast<size_t>(tileWidth * spp * bps / 8);
    std::vector<byte> row(rowSamples);

    const size_t tileCount = tiles.size();
    auto dataPtr = std::vector<byte*>(tileCount);
    for(size_t tile = 0; tile < tileCount; tile++)
    {
      dataPtr[tile] = tiles[tile]->data.get();
    }

    for(size_t i = 0; i < static_cast<size_t>(lineCount); i++)
    {
      for(size_t s = 0; s < rowSamples; s++)
      {
        int value = 0;
        skipComments(m_pnm);
        m_pnm >> value;
        row[s] = static_cast<byte>(value);
      }

      for(size_t tile = 0; tile < tileCount - 1; tile++)
      {
        memcpy(dataPtr[tile], row.data() + (firstTile + tile) * tileStride, tileStride);
        dataPtr[tile] += tileStride;
      }
      memcpy(
        dataPtr[tileCount - 1],
        row.data() + (firstTile + tileCount - 1) * tileStride,
        rowSamples - (firstTile + tileCount - 1) * tileStride
      );
      dataPtr[tileCount - 1] += tileStride;
    }
  }

  ////////////////////////////////////////////////////////////////////////
  // AsciiSource1bpp
  ////////////////////////////////////////////////////////////////////////

  AsciiSource1bpp::Ptr AsciiSource1bpp::create(std::string fileName, std::ifstream pnm, BitmapMetaData bmd)
  {
    return Ptr(new AsciiSource1bpp(std::move(fileName), std::move(pnm), std::move(bmd)));
  }

  void AsciiSource1bpp::fillTiles(int /*startLine*/, int lineCount, int tileWidth, int firstTile_, std::vector<Tile::Ptr>& tiles)
  {
    const auto firstTile = static_cast<size_t>(firstTile_);
    const auto tileStride = static_cast<size_t>(tileWidth / 8);
    const auto width = static_cast<size_t>(m_bmd.rect.width());

    const size_t tileCount = tiles.size();
    auto dataPtr = std::vector<byte*>(tileCount);
    for(size_t tile = 0; tile < tileCount; tile++)
    {
      dataPtr[tile] = tiles[tile]->data.get();
    }

    for(size_t i = 0; i < static_cast<size_t>(lineCount); i++)
    {
      // Read all pixel values for this row, pack into per-tile byte rows MSB-first
      for(size_t x = 0; x < width; x++)
      {
        skipComments(m_pnm);
        char c = '0';
        m_pnm.get(c);
        const int value = (c == '1') ? 1 : 0;

        const size_t tileIndex = (firstTile + x / static_cast<size_t>(tileWidth));
        const size_t localTile = tileIndex - firstTile;
        const size_t bitPos = x % static_cast<size_t>(tileWidth);
        if(value != 0)
        {
          dataPtr[localTile][bitPos / 8] |= static_cast<byte>(0x80u >> (bitPos % 8));
        }
      }

      for(size_t tile = 0; tile < tileCount; tile++)
      {
        dataPtr[tile] += tileStride;
      }
    }
  }

} // namespace Scroom::Pnm
