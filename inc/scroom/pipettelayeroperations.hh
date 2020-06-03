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

/**
 * This strind defines the order in which color channels are stored in the map
 * and eventually in what order the channels will be displayed to the user.
 */
const std::string COLOR_CHANNELS = "CMYK" "RGB" "ADEFHIJLNOPQSTUVWXZ";

/**
 * This struct is just here to compare two keys according to the order defined above
 */
struct ColorComparator {
  bool operator()(const std::string& key_a, const std::string& key_b) const {
    return COLOR_CHANNELS.find(key_a) < COLOR_CHANNELS.find(key_b);
  }
};

class PipetteLayerOperations : public virtual Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<PipetteLayerOperations> Ptr;
  typedef std::map<std::string, size_t, ColorComparator> PipetteColor;

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
