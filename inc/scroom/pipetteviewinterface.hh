/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/interface.hh>
#include <scroom/pipettelayeroperations.hh>
#include <scroom/rectangle.hh>

const std::string PIPETTE_PROPERTY_NAME = "Pipette";

class PipetteViewInterface
  : public virtual Scroom::Utils::Base
  , private Interface
{
public:
  using Ptr = boost::shared_ptr<PipetteViewInterface>;

public:
  /**
   * Returns the average pixel values for each component, contained in the area.
   *
   * @param area selected area to get the pixel values from.
   */
  virtual PipetteLayerOperations::PipetteColor getPixelAverages(Scroom::Utils::Rectangle<double> area) = 0;
};
