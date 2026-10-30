/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/tweak-view.hh>

namespace Scroom::TiledBitmap
{
  class PixelAlignPresentationPositions : public Utils::ITweakPresentationPosition
  {
  public:
    using Ptr = std::shared_ptr<PixelAlignPresentationPositions>;

    static Utils::ITweakPresentationPosition::Ptr create();

    [[nodiscard]] Utils::Point<double> tweakPosition(const Utils::Point<double>& currentPosition, int zoom) const override;
  };

  class SnapSelectionsToGrid : public Utils::TweakSelectionsAsRectangles
  {
  public:
    static Utils::ITweakSelection::Ptr create();

    [[nodiscard]] Utils::Rectangle<double> tweakSelection(const Utils::Rectangle<double>& selectionArea) const override;

    using Utils::TweakSelectionsAsRectangles::tweakSelection;
    using Utils::TweakSelectionsAsRectangles::TweakSelectionsAsRectangles;
  };

  class ExpandSelectionsToPixelBoundaries : public Utils::TweakSelectionsAsRectangles
  {
  public:
    static Utils::ITweakSelection::Ptr create();

    [[nodiscard]] Utils::Rectangle<double> tweakSelection(const Utils::Rectangle<double>& selectionArea) const override;

    using Utils::TweakSelectionsAsRectangles::tweakSelection;
    using Utils::TweakSelectionsAsRectangles::TweakSelectionsAsRectangles;
  };
} // namespace Scroom::TiledBitmap
