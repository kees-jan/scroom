/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <scroom/interface.hh>
#include <scroom/layer-reduction.hh>
#include <scroom/layer-rendering-operations.hh>
#include <scroom/progressinterface.hh>
#include <scroom/stuff.hh>
#include <scroom/tile.hh>

// Compatibility facade for existing operation factories. Data consumers retain
// the independent reducer; renderers retain only LayerRenderingOperations.
class LayerOperations : public LayerRenderingOperations
{
public:
  using Ptr = std::shared_ptr<LayerOperations>;

  [[nodiscard]] virtual int getBpp() const = 0;
  [[nodiscard]] virtual Scroom::TiledBitmap::LayerReduction createReduction() const = 0;
};

// The supplier ensures each reducer's output matches the following layer's
// storage size and renderer. Pixel encodings remain the supplier's responsibility.
struct LayerDescription
{
  int bpp;
  Scroom::TiledBitmap::LayerReduction reduction;
  LayerRenderingOperations::Ptr rendering;

  LayerDescription(int bpp_, Scroom::TiledBitmap::LayerReduction reduction_, LayerRenderingOperations::Ptr rendering_)
    : bpp(bpp_)
    , reduction(std::move(reduction_))
    , rendering(std::move(rendering_))
  {
  }

  explicit LayerDescription(LayerOperations::Ptr operations)
    : bpp(operations->getBpp())
    , reduction(operations->createReduction())
    , rendering(std::move(operations))
  {
  }
};

/**
 * Descriptions for default pyramid construction and rendering.
 * The first description applies to the bottom layer. Each subsequent layer is
 * reduced by a factor of 8; levels beyond the supplied descriptions reuse the last.
 */
using LayerSpec = std::vector<LayerDescription>;

/**
 * Provide bitmap data
 */
class SourcePresentation : private Interface
{
public:
  using Ptr = std::shared_ptr<SourcePresentation>;

public:
  /**
   * Provide bitmap data
   *
   * During population, this function will be called
   * several times. Each time, this function has to fill the provided
   * tiles with bitmap data.
   *
   * The given tiles are horizontally adjacent.
   *
   * @param startLine @c y coordinate of the first line in the set of
   *    tiles
   * @param lineCount The number of valid image rows to fill, at most the tile height
   * @param tileWidth The width of one tile
   * @param firstTile Index of the first tile provided. This means that
   *    the first @c x coordinate is @c firstTile * @c tileWidth
   * @param tiles The tiles that are to be filled.
   */
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) = 0;

  /**
   * Done reading bitmap data
   *
   * Any open files can be closed now
   */
  virtual void done() = 0;

  virtual std::string getName() = 0;
};

namespace Scroom::TiledBitmap
{
  /**
   * Starts bottom-layer loading on the calling thread, typically scheduling
   * decoder work on Sequentially(). The returned registration keeps that work
   * active; releasing it requests cancellation.
   *
   * @see scheduleLoadingBitmap()
   */
  using ReloadFunction = std::function<Scroom::Utils::Stuff(const ProgressInterface::Ptr&)>;

} // namespace Scroom::TiledBitmap
