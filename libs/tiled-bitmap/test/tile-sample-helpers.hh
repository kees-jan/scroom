/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <utility>

#include <scroom/bitmap-helpers.hh>
#include <scroom/rectangle.hh>
#include <scroom/tile.hh>

namespace Scroom::TiledBitmap::Test
{
  namespace RgbChannels
  {
    inline constexpr int RED = 0;
    inline constexpr int GREEN = 1;
    inline constexpr int BLUE = 2;
    inline constexpr int COUNT = 3;
  } // namespace RgbChannels

  namespace CmykChannels
  {
    inline constexpr int CYAN = 0;
    inline constexpr int MAGENTA = 1;
    inline constexpr int YELLOW = 2;
    inline constexpr int BLACK = 3;
    inline constexpr int COUNT = 4;
  } // namespace CmykChannels

  struct SampleLayout
  {
    int bitsPerSample;
    int channels;

    [[nodiscard]] int bitsPerPixel() const { return bitsPerSample * channels; }
  };

  inline Tile::Ptr emptyTile(int size, SampleLayout format)
  {
    const int byteCount = size * size * format.bitsPerPixel() / 8;
    Scroom::MemoryBlobs::RawPageData::Ptr data(static_cast<uint8_t*>(calloc(byteCount, 1)), free);
    return Tile::create(size, size, format.bitsPerPixel(), std::move(data));
  }

  inline ConstTile::Ptr readOnly(const Tile::Ptr& tile)
  {
    return ConstTile::create(tile->width, tile->height, tile->bpp, tile->data);
  }

  template <typename Base>
  inline void setSample(const Tile::Ptr& tile, SampleLayout format, int x, int y, int channel, unsigned value)
  {
    const int index = (y * tile->width + x) * format.channels + channel;
    Scroom::Bitmap::SampleIterator<Base> iterator(reinterpret_cast<Base*>(tile->data.get()), index, format.bitsPerSample);
    iterator.set(static_cast<Base>(value));
  }

  template <typename Base>
  inline unsigned sample(const Tile::Ptr& tile, SampleLayout format, int x, int y, int channel)
  {
    const int index = (y * tile->width + x) * format.channels + channel;
    Scroom::Bitmap::SampleIterator<const Base> iterator(
      reinterpret_cast<const Base*>(tile->data.get()), index, format.bitsPerSample
    );
    return *iterator;
  }

  inline void setSample(const Tile::Ptr& tile, SampleLayout format, int x, int y, int channel, unsigned value)
  {
    if(format.bitsPerSample == 16)
    {
      setSample<uint16_t>(tile, format, x, y, channel, value);
    }
    else
    {
      setSample<uint8_t>(tile, format, x, y, channel, value);
    }
  }

  inline unsigned sample(const Tile::Ptr& tile, SampleLayout format, int x, int y, int channel)
  {
    if(format.bitsPerSample == 16)
    {
      return sample<uint16_t>(tile, format, x, y, channel);
    }
    return sample<uint8_t>(tile, format, x, y, channel);
  }


  template <typename Base>
  inline void
    fillChannel(const Tile::Ptr& tile, SampleLayout format, Scroom::Utils::Rectangle<int> area, int channel, unsigned value)
  {
    for(int y = area.getTop(); y < area.getBottom(); ++y)
    {
      for(int x = area.getLeft(); x < area.getRight(); ++x)
      {
        setSample<Base>(tile, format, x, y, channel, value);
      }
    }
  }

  inline void
    fillChannel(const Tile::Ptr& tile, SampleLayout format, Scroom::Utils::Rectangle<int> area, int channel, unsigned value)
  {
    if(format.bitsPerSample == 16)
    {
      fillChannel<uint16_t>(tile, format, area, channel, value);
    }
    else
    {
      fillChannel<uint8_t>(tile, format, area, channel, value);
    }
  }

  inline Scroom::Utils::Rectangle<int> tileArea(const Tile::Ptr& tile) { return {0, 0, tile->width, tile->height}; }

  inline unsigned firstSample(const Tile::Ptr& tile, SampleLayout format) { return sample(tile, format, 0, 0, 0); }

} // namespace Scroom::TiledBitmap::Test
