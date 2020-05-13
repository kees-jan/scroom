/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/layeroperations.hh>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <glib.h>

#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <scroom/bitmap-helpers.hh>
#include <scroom/cairo-helpers.hh>
#include <scroom/unused.hh>

#include <boost/math/special_functions/round.hpp>
#include <iostream>

////////////////////////////////////////////////////////////////////////

namespace
{
  boost::shared_ptr<unsigned char> shared_malloc(size_t size)
  {
    return boost::shared_ptr<unsigned char>(static_cast<unsigned char*>(malloc(size)), free);
  }

}

////////////////////////////////////////////////////////////////////////
// OperationsCMYK

LayerOperations::Ptr OperationsCMYK::create()
{
  return Ptr(new OperationsCMYK());
}

OperationsCMYK::OperationsCMYK()
{
}

int OperationsCMYK::getBpp()
{
  // CMYK has 4 channels -> 4 samples per pixel.
  return 32;
}

// From https://www.pagetable.com/?p=23#comment-1140
inline uint16_t DivideBy255(uint16_t value)
{
  return static_cast<uint16_t>((value + 1 + (value >> 8)) >> 8);
}

/**
 * Cache the given tile
 */
Scroom::Utils::Stuff OperationsCMYK::cache(const ConstTile::Ptr tile)
{
  // Allocate the space for the cache - stride is the height of one row
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
  boost::shared_ptr<uint8_t> data = shared_malloc(static_cast<size_t>(stride * tile->height));

  // Row is a pointer to a row of pixels (destination)
  uint32_t* row = reinterpret_cast<uint32_t*>(data.get());
  // Cur is a pointer to the start of the row in the tile (source)
  const uint8_t* cur = tile->data.get();

  // assume stride = tile->width * 4
  for (int i = 0; i < 4 * tile->height * tile->width; i += 4)
  {
    // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
    uint8_t C_i = static_cast<uint8_t>(255 - cur[i + 0]);
    uint8_t M_i = static_cast<uint8_t>(255 - cur[i + 1]);
    uint8_t Y_i = static_cast<uint8_t>(255 - cur[i + 2]);
    uint8_t K_i = static_cast<uint8_t>(255 - cur[i + 3]);

    uint32_t R = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(C_i * K_i)));
    uint32_t G = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(M_i * K_i)));
    uint32_t B = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(Y_i * K_i)));

    // Write 255 as alpha (fully opaque)
    row[i / 4] = 255u << 24 | R << 16 | G << 8 | B;
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}

void OperationsCMYK::reduce(Tile::Ptr target, const ConstTile::Ptr source, int top_left_x, int top_left_y)
{
  // Reducing by a factor 8. Source tile is 24bpp. Target tile is 24bpp
  int sourceStride = 4 * source->width; // stride in bytes
  const byte* sourceBase = source->data.get();

  int targetStride = 4 * target->width; // stride in bytes
  byte* targetBase = target->data.get() +
      (target->height * top_left_y + top_left_x) * targetStride / 8;

  for (int y = 0; y < source->height / 8; y++)
  {
    const byte* sourcePtr = sourceBase;
    byte* targetPtr = targetBase;

    for (int x = 0; x < source->width / 8; x++)
    {
      // We want to store the average colour of the 8*8 pixel image
      // with (x, y) as its top-left corner into targetPtr.
      const byte* base = sourcePtr;
      int sum_c = 0;
      int sum_m = 0;
      int sum_y = 0;
      int sum_k = 0;
      for (int k = 0; k < 8; k++, base += sourceStride)
      {
        const byte* current = base;
        for (int l = 0; l < 8; l++)
        {
          sum_c += current[0];
          sum_m += current[1];
          sum_y += current[2];
          sum_k += current[3];
          current += 4;
        }
      }

      targetPtr[0] = static_cast<byte>(sum_c / 64);
      targetPtr[1] = static_cast<byte>(sum_m / 64);
      targetPtr[2] = static_cast<byte>(sum_y / 64);
      targetPtr[3] = static_cast<byte>(sum_k / 64);

      sourcePtr += 8 * 4;
      targetPtr += 4;
    }

    targetBase += targetStride;
    sourceBase += sourceStride * 8;
  }
}