/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "tweak-view.hh"

#include <scroom/cairo-helpers.hh>

namespace Scroom::TiledBitmap
{
  using namespace Scroom::Utils;

  ITweakPresentationPosition::Ptr PixelAlignPresentationPositions::create() { return Ptr(new PixelAlignPresentationPositions()); }

  Point<double> PixelAlignPresentationPositions::tweakPosition(const Point<double>& currentPosition, int zoom) const
  {
    return round_to_multiple_of(currentPosition, make_point(1.0) / pixelSizeFromZoom(zoom));
  }

  ITweakSelection::Ptr SnapSelectionsToGrid::create() { return Ptr(new SnapSelectionsToGrid()); }

  Rectangle<double> SnapSelectionsToGrid::tweakSelection(const Rectangle<double>& selectionArea) const
  {
    return roundCorners(selectionArea);
  }

  ITweakSelection::Ptr ExpandSelectionsToPixelBoundaries::create() { return Ptr(new ExpandSelectionsToPixelBoundaries()); }

  Rectangle<double> ExpandSelectionsToPixelBoundaries::tweakSelection(const Rectangle<double>& selectionArea) const
  {
    return roundOutward(selectionArea);
  }

} // namespace Scroom::TiledBitmap
