/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <string>
#include <scroom/rectangle.hh>
#include <scroom/tile.hh>

class PipetteLayerOperations : public virtual Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<PipetteLayerOperations> Ptr;
  typedef std::vector<std::pair<std::string, double>> PipetteColor;

public:
  virtual ~PipetteLayerOperations()
  {}

  /**
   * Sums the samples of each pixel contained in the area of the tile.
   * 
   * @param area The rectangular area in which pixels are summed up.
   * @param tile The tile in which the pixels are located in.
   */
  virtual PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr tile)=0;
};
