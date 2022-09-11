/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>
#include <utility>

#include <scroom/global.hh>
#include <scroom/memoryblobs.hh>

class Tile
{
public:
  using Ptr     = std::shared_ptr<Tile>;
  using WeakPtr = std::weak_ptr<Tile>;

  int                                   width;
  int                                   height;
  int                                   bpp;
  Scroom::MemoryBlobs::RawPageData::Ptr data;

public:
  Tile(int width_, int height_, int bpp_, Scroom::MemoryBlobs::RawPageData::Ptr data_)
    : width(width_)
    , height(height_)
    , bpp(bpp_)
    , data(std::move(data_))
  {
  }

  static Tile::Ptr create(int width, int height, int bpp, Scroom::MemoryBlobs::RawPageData::Ptr data)
  {
    return std::make_shared<Tile>(width, height, bpp, std::move(data));
  }
};

class ConstTile
{
public:
  using Ptr     = std::shared_ptr<ConstTile>;
  using WeakPtr = std::weak_ptr<ConstTile>;

  int                                        width;
  int                                        height;
  int                                        bpp;
  Scroom::MemoryBlobs::RawPageData::ConstPtr data;

public:
  ConstTile(int width_, int height_, int bpp_, Scroom::MemoryBlobs::RawPageData::ConstPtr data_)
    : width(width_)
    , height(height_)
    , bpp(bpp_)
    , data(std::move(data_))
  {
  }

  static ConstTile::Ptr create(int width, int height, int bpp, Scroom::MemoryBlobs::RawPageData::ConstPtr data)
  {
    return std::make_shared<ConstTile>(width, height, bpp, std::move(data));
  }
};
