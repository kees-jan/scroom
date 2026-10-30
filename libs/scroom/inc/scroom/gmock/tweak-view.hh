/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <gmock/gmock.h>

#include <scroom/tweak-view.hh>

namespace Scroom::Utils
{
  class PresentationPositionTweakMock : public ITweakPresentationPosition
  {
  public:
    MOCK_METHOD(Point<double>, tweakPosition, (const Point<double>&, int), (const, override));
  };

  class RulerAreaTweakMock : public ITweakRulers
  {
  public:
    MOCK_METHOD(Rectangle<double>, tweakRulers, (const Point<double>&, const Point<int>&, int), (const, override));
  };

  class SelectionTweakMock : public ITweakSelection
  {
  public:
    MOCK_METHOD(Selection, tweakSelection, (const Selection&), (const, override));
  };

  class ViewCentreTextConversionMock : public ITweakPositionTextBox
  {
  public:
    MOCK_METHOD(Point<double>, parse, (std::string_view, std::string_view, const Point<int>&, int), (const, override));
    MOCK_METHOD(
      (std::pair<std::string, std::string>),
      display,
      (const Point<double>&, const Point<int>&, int),
      (const, override)
    );
  };

  class TweakSelectionsAsRectanglesMock : public TweakSelectionsAsRectangles
  {
  public:
    using TweakSelectionsAsRectangles::tweakSelection;
    MOCK_METHOD(Rectangle<double>, tweakSelection, (const Rectangle<double>&), (const, override));
  };
} // namespace Scroom::Utils
