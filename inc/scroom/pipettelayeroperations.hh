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
  typedef std::map<std::string, size_t> PipetteColor;

public:
  virtual ~PipetteLayerOperations()
  {}

  /**
   * Method that each layer Operation that supports the pipette tool will implement.
   * This will, given a tile and an area, where the area is contained in the tile, compute the sum of all components.
   * A map of strings and size_t will then be returned, representing the sum for each component.
   */
  virtual PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr tile)=0;
};
