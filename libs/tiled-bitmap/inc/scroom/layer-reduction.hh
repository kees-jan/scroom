/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <functional>

#include <scroom/tile.hh>

namespace Scroom::TiledBitmap
{
  /**
   * Reduce a full source tile by a factor of 8 into one destination subregion.
   *
   * @param[in,out] target Destination tile; only the selected subregion is written.
   * @param[in] source Source tile, including zero-filled samples outside the image.
   * @param[in] x Horizontal destination subregion index in [0, 8).
   * @param[in] y Vertical destination subregion index in [0, 8).
   *
   * Source and destination are equally sized square buffers with tightly packed
   * rows and sizes divisible by 8. Packed output subregions start on byte
   * boundaries. Zero-filled boundary samples participate in reduction. The caller
   * retains the buffers and coordinates concurrent writes to the destination.
   * Each consumer owns a copy of the callable and its captured parameters.
   */
  using LayerReduction = std::function<void(Tile::Ptr target, ConstTile::Ptr source, int x, int y)>;

  // Built-in reducers for the existing bitmap operations. Plugins can supply
  // their own reduction callables for other pixel encodings.
  // Colormap pairs contain two indices per pixel, packed most-significant first
  // in native uint16_t words. Other packed samples use bytes; 16-bit samples
  // use native byte order.
  LayerReduction createIntensityReduction(int bitsPerSample);
  LayerReduction createRgbReduction(int bitsPerSample);
  LayerReduction createCmykReduction(int bitsPerSample);
  LayerReduction createColormapReduction(int bitsPerIndex);
  LayerReduction createColormapPairReduction(int bitsPerIndex);
  LayerReduction createBinaryOrReduction();
} // namespace Scroom::TiledBitmap
