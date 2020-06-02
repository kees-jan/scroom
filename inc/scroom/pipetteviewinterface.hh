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

  virtual PipetteLayerOperations::PipetteColor getAverages(Scroom::Utils::Rectangle<int> area)=0;
};