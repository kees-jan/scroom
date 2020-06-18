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
 * This string defines the order in which color channels are stored in the map
 * and eventually in what order the channels will be displayed to the user.
 */
const std::string COLOR_CHANNELS = "CMYK" "RGB" "ADEFHIJLNOPQSTUVWXZ";

/**
 * This struct is just here to compare two keys according to the order defined above
 */
struct ColorComparator : public std::binary_function<std::string, std::string, bool> {
public:
  bool operator()(const std::string& key_a, const std::string& key_b) const {
    return COLOR_CHANNELS.find(key_a) < COLOR_CHANNELS.find(key_b);
  }
};

class PipetteLayerOperations : public virtual Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<PipetteLayerOperations> Ptr;
  typedef std::map<std::string, double, ColorComparator> PipetteColor;

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
