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

LayerOperations::Ptr OperationsCMYK::create(size_t bps) {
  return Ptr(new OperationsCMYK(bps));
}

OperationsCMYK::OperationsCMYK(size_t bps_) : bps(bps_) { }

int OperationsCMYK::getBpp() {
  // CMYK has 4 channels -> 4 samples per pixel.
  return static_cast<int>(this->bps * 4);
}

/**
 * Cache the given tile
 */
Scroom::Utils::Stuff OperationsCMYK::cache(const ConstTile::Ptr tile) {
    // Allocate the space for the cache - stride is the height of one row
    const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
    boost::shared_ptr<uint8_t> data = shared_malloc(static_cast<size_t>(stride * tile->height));

    // Row is a pointer to a row of pixels (destination)
    uint32_t* row = reinterpret_cast<uint32_t*>(data.get());
    // Cur is a pointer to the start of the row in the tile (source)
    const uint32_t* cur = reinterpret_cast<const uint32_t*>(tile->data.get());

    // assume stride = tile->width * 4
    for (int i = 0; i < tile->height * tile->width; i++) {
        // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
        uint32_t CMYK = cur[i];

        // Not sure why the order of CMYK is reversed...
        uint32_t C =  CMYK        & 0xFF;
        uint32_t M = (CMYK >>  8) & 0xFF;
        uint32_t Y = (CMYK >> 16) & 0xFF;
        uint32_t K = (CMYK >> 24) & 0xFF;
        float K_i = 1.0f - (K / 255.0f);

        uint32_t R = boost::math::iround((255.0f - C) * K_i);
        uint32_t G = boost::math::iround((255.0f - M) * K_i);
        uint32_t B = boost::math::iround((255.0f - Y) * K_i);

        // Write 255 as alpha (fully opaque)
        row[i] = 0xFFu << 24 | R << 16 | G << 8 | B;
    }

    return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}

void OperationsCMYK::reduce(Tile::Ptr target, const ConstTile::Ptr source, int x_, int y_) {
    // Reducing by a factor 8. Source tile is 24bpp. Target tile is 24bpp
    int sourceStride = 3 * source->width; // stride in bytes
    const byte* sourceBase = source->data.get();

    int targetStride = 3 * target->width; // stride in bytes
    byte* targetBase = target->data.get() +
        (target->height * y_ + x_) * targetStride / 8;

    for (int y = 0; y < source->height; y += 8) {
        const byte* sourcePtr = sourceBase + 8 * y * sourceStride;
        byte* targetPtr = targetBase + y * targetStride;

        for (int x = 0; x < source->width; x += 8) {
            // We want to store the average colour of the 8*8 pixel image
            // with (x, y) as its top-left corner into targetPtr.
            const byte* base = sourcePtr;
            unsigned int sum_r = 0;
            unsigned int sum_g = 0;
            unsigned int sum_b = 0;

            for (int k = 0; k < 8; k++, base += sourceStride) {
                const byte* current = base;
                for (int l = 0; l < 8; l++) {
                    sum_r += current[0];
                    sum_g += current[1];
                    sum_b += current[2];
                    current += 3;
                }
            }

            targetPtr[0] = (sum_r / 64u) & 0xFFu;
            targetPtr[1] = (sum_g / 64u) & 0xFFu;
            targetPtr[2] = (sum_b / 64u) & 0xFFu;

            sourcePtr += 8 * 3;
            targetPtr += 3;
        }
    }
}