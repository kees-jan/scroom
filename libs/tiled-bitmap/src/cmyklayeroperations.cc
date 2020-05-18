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

LayerOperations::Ptr OperationsCMYK::create(uint16_t bps)
{
  return Ptr(new OperationsCMYK(bps));
}

OperationsCMYK::OperationsCMYK(uint16_t bps_)
  : bps(bps_)
{
}

int OperationsCMYK::getBpp()
{
  // CMYK has 4 channels -> 4 samples per pixel.
  return this->bps * 4;
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
  if (this->bps == 8) {
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
  } else if (this->bps == 4) {
    for (int i = 0; i < 2 * tile->height * tile->width; i += 2)
    {
      // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
      uint8_t C_i = static_cast<uint8_t>(255 - ((cur[i + 0]       ) >> 4) * 17); // 17 == 255/15
      uint8_t M_i = static_cast<uint8_t>(255 - ((cur[i + 0] & 0x0F)     ) * 17);
      uint8_t Y_i = static_cast<uint8_t>(255 - ((cur[i + 1]       ) >> 4) * 17);
      uint8_t K_i = static_cast<uint8_t>(255 - ((cur[i + 1] & 0x0F)     ) * 17);

      uint32_t R = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(C_i * K_i)));
      uint32_t G = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(M_i * K_i)));
      uint32_t B = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(Y_i * K_i)));

      // Write 255 as alpha (fully opaque)
      row[i / 2] = 255u << 24 | R << 16 | G << 8 | B;
    }
  } else if (this->bps == 2) {
    for (int i = 0; i < tile->height * tile->width; i++)
    {
      // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
      uint8_t C_i = static_cast<uint8_t>(255 - ((cur[i]       ) >> 6) * 85); // 85 == 255/3
      uint8_t M_i = static_cast<uint8_t>(255 - ((cur[i] & 0x30) >> 4) * 85);
      uint8_t Y_i = static_cast<uint8_t>(255 - ((cur[i] & 0x0C) >> 2) * 85);
      uint8_t K_i = static_cast<uint8_t>(255 - ((cur[i] & 0x03)     ) * 85);

      uint32_t R = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(C_i * K_i)));
      uint32_t G = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(M_i * K_i)));
      uint32_t B = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(Y_i * K_i)));

      // Write 255 as alpha (fully opaque)
      row[i] = 255u << 24 | R << 16 | G << 8 | B;
    }
  } else if (this->bps == 1) {
    for (int i = 0; i < tile->height * tile->width; i++)
    {
      // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
      uint8_t C_i, M_i, Y_i, K_i;
      if ((i & 1) == 0) { // even pixels -> top half of byte
        C_i = static_cast<uint8_t>(((cur[i / 2]       ) >> 7) - 1); // 0 -> 255 (= -1), 1 -> 0
        M_i = static_cast<uint8_t>(((cur[i / 2] & 0x40) >> 6) - 1);
        Y_i = static_cast<uint8_t>(((cur[i / 2] & 0x20) >> 5) - 1);
        K_i = static_cast<uint8_t>(((cur[i / 2] & 0x10) >> 4) - 1);
      } else { // odd pixels -> lower half of the byte
        C_i = static_cast<uint8_t>(((cur[i / 2] & 0x08) >> 3) - 1); // 0 -> 255 (= -1), 1 -> 0
        M_i = static_cast<uint8_t>(((cur[i / 2] & 0x04) >> 2) - 1);
        Y_i = static_cast<uint8_t>(((cur[i / 2] & 0x02) >> 1) - 1);
        K_i = static_cast<uint8_t>(((cur[i / 2] & 0x01)     ) - 1);
      }

      uint32_t R_1 = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(C_i * K_i)));
      uint32_t G_1 = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(M_i * K_i)));
      uint32_t B_1 = static_cast<uint8_t>(DivideBy255(static_cast<uint16_t>(Y_i * K_i)));

      // Write 255 as alpha (fully opaque)
      row[i] = 255u << 24 | R_1 << 16 | G_1 << 8 | B_1;
    }
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}

// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
uint8_t bitcount(uint32_t c) {
  c = c - ((c >> 1) & 0x55555555);
  c = ((c >> 2) & 0x33333333) + (c & 0x33333333);
  c = ((c >> 4) + c) & 0x0F0F0F0F;
  c = ((c >> 8) + c) & 0x00FF00FF;
  return ((c >> 16) + c) & 0x0000FFFF;
}

void OperationsCMYK::reduce(Tile::Ptr target, const ConstTile::Ptr source, int top_left_x, int top_left_y)
{
  // Reducing by a factor 8
  int sourceStride = this->bps * source->width / 2; // stride in bytes
  const byte* sourceBase = source->data.get();

  int targetStride = this->bps * target->width / 2; // stride in bytes
  byte* targetBase = target->data.get() +
      (target->height * top_left_y + top_left_x) * targetStride / 8;
  
  if (this->bps > 1) {
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
          size_t current = 0;
          for (int l = 0; l < 8; l++)
          {
            if (this->bps == 8) {
              sum_c += base[current + 0];
              sum_m += base[current + 1];
              sum_y += base[current + 2];
              sum_k += base[current + 3];
              current += 4;
            } else if (this->bps == 4) {
              sum_c += base[current + 0] >> 4;
              sum_m += base[current + 0] & 15;
              sum_y += base[current + 1] >> 4;
              sum_k += base[current + 1] & 15;
              current += 2;
            } else if (this->bps == 2) {
              sum_c +=  base[current] >> 6;
              sum_m += (base[current] & 0x30) >> 4;
              sum_y += (base[current] & 0x0C) >> 2;
              sum_k +=  base[current] & 0x03;
              current++;
            }
          }
        }

        if (this->bps == 8) {
          targetPtr[0] = static_cast<byte>(sum_c / 64);
          targetPtr[1] = static_cast<byte>(sum_m / 64);
          targetPtr[2] = static_cast<byte>(sum_y / 64);
          targetPtr[3] = static_cast<byte>(sum_k / 64);
        } else if (this->bps == 4) {
          targetPtr[0] = static_cast<byte>(sum_c / 64) << 4 | static_cast<byte>((sum_m / 64) & 0xF);
          targetPtr[1] = static_cast<byte>(sum_y / 64) << 4 | static_cast<byte>((sum_k / 64) & 0xF);
        } else if (this->bps == 2) {
          targetPtr[0] = static_cast<byte>(sum_c / 64) << 6 | 
                        static_cast<byte>(((sum_m / 64) & 0x3) << 4) |
                        static_cast<byte>(((sum_y / 64) & 0x3) << 2) |
                        static_cast<byte>((sum_k / 64) & 0x3);
        }

        sourcePtr += 4 * this->bps;
        targetPtr += this->bps / 2;
      }

      targetBase += targetStride;
      sourceBase += sourceStride * 8;
    }
  } else {
    // 1 bps
    for (int y = 0; y < source->height / 8; y++)
    {
      const uint32_t* sourcePtr = reinterpret_cast<const uint32_t*>(sourceBase);

      for (int x = 0; x < source->width / 8; x++)
      {
        // Find average of 8x8 pixel area
        // We don't care about the order of pixels, because
        // addition is associative.
        uint32_t row = sourceBase[x];
        uint8_t sum_c = bitcount(row & 0b10001000100010001000100010001000);
        uint8_t sum_m = bitcount(row & 0b01000100010001000100010001000100);
        uint8_t sum_y = bitcount(row & 0b00100010001000100010001000100010);
        uint8_t sum_k = bitcount(row & 0b00010001000100010001000100010001);

        // Since a single pixel takes up half a byte, we need to do some
        // bitshifts to get the bits in the right positions.
        if ((x & 1) == 0) {
          targetBase[x/2] = ((sum_c & 4) << 5) \
                       | ((sum_m & 4) << 4) \
                       | ((sum_y & 4) << 3) \
                       | ((sum_k & 4) << 2);
        } else {
          targetBase[x/2] |= ((sum_c & 4) << 1) \
                       | ((sum_m & 4)     ) \
                       | ((sum_y & 4) >> 1) \
                       | ((sum_k & 4) >> 2);
        }
      }

      targetBase += targetStride;
      sourceBase += sourceStride * 8;
    }
  }
}
