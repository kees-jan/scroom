/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>

#include <scroom/interface.hh>
#include <scroom/point.hh>
#include <scroom/rectangle.hh>
#include <scroom/viewinterface.hh>

namespace Scroom::Utils
{
  class ITweakPresentationPosition : public Interface
  {
  public:
    using Ptr = std::shared_ptr<ITweakPresentationPosition>;

    [[nodiscard]] virtual Point<double>
      tweakPosition(Point<double> currentPosition, Point<int> drawingAreaSize, int zoom) const = 0;
  };

  class ITweakRulers : public Interface
  {
  public:
    using Ptr = std::shared_ptr<ITweakRulers>;

    [[nodiscard]] virtual Rectangle<double>
      tweakRulers(Point<double> currentPosition, Point<int> drawingAreaSize, int zoom) const = 0;
  };

  class ITweakSelection : public Interface
  {
  public:
    using Ptr = std::shared_ptr<ITweakSelection>;

    [[nodiscard]] virtual Selection tweakSelection(Selection selection) const = 0;
  };

  class ITweakPositionTextBox : public Interface
  {
  public:
    using Ptr = std::shared_ptr<ITweakPositionTextBox>;

    [[nodiscard]] virtual Point<double>
      parse(std::string_view x, std::string_view y, Point<int> drawingAreaSize, int zoom) const = 0;

    [[nodiscard]] virtual std::pair<std::string, std::string>
      display(Point<double> position, Point<int> drawingAreaSize, int zoom) const = 0;
  };

  ITweakPresentationPosition::Ptr getDefaultTweakPresentationPosition();
  ITweakRulers::Ptr               getDefaultTweakRulers();
  ITweakSelection::Ptr            getDefaultTweakSelection();
  ITweakPositionTextBox::Ptr      getDefaultTweakPositionTextBox();

} // namespace Scroom::Utils