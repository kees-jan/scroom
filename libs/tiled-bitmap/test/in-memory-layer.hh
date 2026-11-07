/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstdlib>
#include <mutex>
#include <vector>

#include <scroom/assertions.hh>
#include <scroom/layer-base.hh>
#include <scroom/tiled-bitmap-geometry.hh>

namespace Scroom::TiledBitmap::Test
{
  class InMemoryTile : public TileStorage
  {
  private:
    int m_bpp;
    bool m_outOfBounds;
    Tile::Ptr m_pixels;
    std::mutex m_mutex;

  public:
    explicit InMemoryTile(int bpp, bool outOfBounds = false)
      : m_bpp(bpp)
      , m_outOfBounds(outOfBounds)
    {
    }

    int getBpp() const override { return m_bpp; }

    Utils::Stuff initialize() override
    {
      std::lock_guard lock(m_mutex);
      if(!m_pixels)
      {
        const auto byteCount = static_cast<std::size_t>(TILESIZE) * TILESIZE * m_bpp / 8;
        std::shared_ptr<uint8_t> data(static_cast<uint8_t*>(calloc(byteCount, 1)), free);
        m_pixels = Tile::create(TILESIZE, TILESIZE, m_bpp, std::move(data));
      }
      return m_pixels;
    }

    Tile::Ptr getTileSync() override
    {
      std::lock_guard lock(m_mutex);
      return m_pixels;
    }

    ConstTile::Ptr getConstTileSync() override
    {
      std::lock_guard lock(m_mutex);
      return m_pixels ? ConstTile::create(TILESIZE, TILESIZE, m_bpp, m_pixels->data) : ConstTile::Ptr{};
    }

    ConstTile::Ptr getConstTileAsync() override { return getConstTileSync(); }

    TileState getState() override
    {
      std::lock_guard lock(m_mutex);
      return m_outOfBounds ? TILE_OUT_OF_BOUNDS : (m_pixels ? TILE_LOADED : TILE_UNINITIALIZED);
    }
  };

  class InMemoryLayer : public LayerBase
  {
  private:
    Utils::Rectangle<int> m_imageArea;
    Utils::Rectangle<int> m_tileRange;
    int m_bpp;
    std::vector<TileStorage::Ptr> m_tiles;
    TileStorage::Ptr m_outOfBounds;

  public:
    InMemoryLayer(Utils::Point<int> imageSize, int bpp)
      : m_imageArea(0, 0, imageSize.x, imageSize.y)
      , m_tileRange(Geometry::enclosingTileRange(m_imageArea, TILESIZE))
      , m_bpp(bpp)
      , m_outOfBounds(std::make_shared<InMemoryTile>(bpp, true))
    {
      for(const auto index [[maybe_unused]]: m_tileRange)
      {
        m_tiles.emplace_back(std::make_shared<InMemoryTile>(bpp));
      }
    }

    Utils::Rectangle<int> getRect() const override { return m_imageArea; }
    Utils::Rectangle<int> getTileRange() const override { return m_tileRange; }
    int getBpp() const override { return m_bpp; }

    TileStorage::Ptr getTile(Utils::Point<int> index) override
    {
      if(!m_tileRange.contains(index))
      {
        return m_outOfBounds;
      }
      return m_tiles[index.y * m_tileRange.getWidth() + index.x];
    }

    TileStorage::Ptr getOutOfBoundTile() override { return m_outOfBounds; }

    static LayerFactory factory()
    {
      return [](int /*depth*/, Utils::Rectangle<int> imageArea, int bpp)
      {
        require(imageArea.getTopLeft() == Utils::Point(0, 0));
        return std::make_shared<InMemoryLayer>(imageArea.getSize(), bpp);
      };
    }
  };
} // namespace Scroom::TiledBitmap::Test
