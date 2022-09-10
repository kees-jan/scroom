/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <string>

#include <scroom/interface.hh>
#include <scroom/rectangle.hh>
#include <scroom/tile.hh>

class PipetteLayerOperations
  : public virtual Scroom::Utils::Base
  , private Interface
{
public:
  using Ptr          = std::shared_ptr<PipetteLayerOperations>;
  using PipetteColor = std::vector<std::pair<std::string, double>>;

public:
  /**
   * Sums the samples of each pixel contained in the area of the tile.
   *
   * @param area The rectangular area in which pixels are summed up.
   * @param tile The tile in which the pixels are located in.
   */
  virtual PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr& tile) = 0;
};
