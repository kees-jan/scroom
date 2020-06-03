/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/pipettelayeroperations.hh>
#include <scroom/rectangle.hh>

class PipetteViewInterface : public virtual Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<PipetteViewInterface> Ptr;

public:
  virtual ~PipetteViewInterface()
  {}

  /** 
   * Method to get the average pixel values in the area.
   * The method assumes that the selected area is completely contained in the presentation.
   * Returns a map of strings and size_t, representing averages for each component.
   */
  virtual PipetteLayerOperations::PipetteColor getAverages(Scroom::Utils::Rectangle<int> area)=0;
};