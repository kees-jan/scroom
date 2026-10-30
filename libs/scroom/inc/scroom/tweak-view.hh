/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

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

    [[nodiscard]] virtual Point<double> tweakPosition(const Point<double>& currentPosition, int zoom) const = 0;
  };

  class ITweakRulers : public Interface
  {
  public:
    using Ptr = std::shared_ptr<ITweakRulers>;

    [[nodiscard]] virtual Rectangle<double>
      tweakRulers(const Point<double>& currentPosition, const Point<int>& viewSize, int zoom) const = 0;
  };

  class ITweakSelection : public Interface
  {
  public:
    using Ptr = std::shared_ptr<ITweakSelection>;
    using Map = std::map<std::string, ITweakSelection::Ptr>;

    [[nodiscard]] virtual Selection tweakSelection(const Selection& selection) const = 0;
  };

  class ITweakPositionTextBox : public Interface
  {
  public:
    using Ptr = std::shared_ptr<ITweakPositionTextBox>;

    [[nodiscard]] virtual Point<double>
      parse(std::string_view x, std::string_view y, const Point<int>& viewSize, int zoom) const = 0;

    [[nodiscard]] virtual std::pair<std::string, std::string>
      display(const Point<double>& position, const Point<int>& viewSize, int zoom) const = 0;
  };

  ITweakPresentationPosition::Ptr getDefaultTweakPresentationPosition();
  ITweakRulers::Ptr getDefaultTweakRulers();
  ITweakSelection::Ptr getDefaultTweakSelection();
  ITweakSelection::Map getDefaultTweakSelectionMap();
  ITweakPositionTextBox::Ptr getDefaultTweakPositionTextBox();

  ////////////////////////////////////////////////////////////////////////

  class CalculateViewAreaForRulers : public ITweakRulers
  {
  public:
    using Ptr = std::shared_ptr<CalculateViewAreaForRulers>;

    static ITweakRulers::Ptr create();

    [[nodiscard]] Rectangle<double>
      tweakRulers(const Point<double>& currentPosition, const Point<int>& viewSize, int zoom) const override;
  };

  class TweakSelectionsAsRectangles : public ITweakSelection
  {
  public:
    using Ptr = std::shared_ptr<TweakSelectionsAsRectangles>;

    [[nodiscard]] virtual Rectangle<double> tweakSelection(const Rectangle<double>& selectionArea) const = 0;

    [[nodiscard]] Selection tweakSelection(const Selection& selection) const override;
  };

  class ParseAndFormatViewCentre : public ITweakPositionTextBox
  {
  public:
    using Ptr = std::shared_ptr<ParseAndFormatViewCentre>;

    static ITweakPositionTextBox::Ptr create();

    [[nodiscard]] Point<double>
      parse(std::string_view x, std::string_view y, const Point<int>& viewSize, int zoom) const override;

    [[nodiscard]] std::pair<std::string, std::string>
      display(const Point<double>& position, const Point<int>& viewSize, int zoom) const override;
  };

  // Decorates the child's coordinate policy; aspect ratios are fixed at construction.
  class TweakPresentationPositionsWithAspectRatio : public ITweakPresentationPosition
  {
  private:
    ITweakPresentationPosition::Ptr m_child;
    const Point<double> m_aspectRatio;

  public:
    static ITweakPresentationPosition::Ptr create(ITweakPresentationPosition::Ptr child, const Point<double>& aspectRatio);
    [[nodiscard]] Point<double> tweakPosition(const Point<double>& currentPosition, int zoom) const override;

  private:
    TweakPresentationPositionsWithAspectRatio(ITweakPresentationPosition::Ptr child, const Point<double>& aspectRatio);
  };

  // Decorates the child's coordinate policy; aspect ratios are fixed at construction.
  class TweakRulersWithAspectRatio : public ITweakRulers
  {
  private:
    ITweakRulers::Ptr m_child;
    const Point<double> m_aspectRatio;

  public:
    static ITweakRulers::Ptr create(ITweakRulers::Ptr child, const Point<double>& aspectRatio);
    [[nodiscard]] Rectangle<double>
      tweakRulers(const Point<double>& currentPosition, const Point<int>& viewSize, int zoom) const override;

  private:
    TweakRulersWithAspectRatio(ITweakRulers::Ptr child, const Point<double>& aspectRatio);
  };

  // Decorates the child's coordinate policy; aspect ratios are fixed at construction.
  class TweakSelectionsWithAspectRatio : public ITweakSelection
  {
  private:
    ITweakSelection::Ptr m_child;
    const Point<double> m_aspectRatio;

  public:
    static ITweakSelection::Ptr create(ITweakSelection::Ptr child, const Point<double>& aspectRatio);
    [[nodiscard]] Selection tweakSelection(const Selection& selection) const override;

  private:
    TweakSelectionsWithAspectRatio(ITweakSelection::Ptr child, const Point<double>& aspectRatio);
  };

  // Decorates the child's coordinate policy; aspect ratios are fixed at construction.
  class ParseAndFormatViewCentreWithAspectRatio : public ITweakPositionTextBox
  {
  private:
    ITweakPositionTextBox::Ptr m_child;
    const Point<double> m_aspectRatio;

  public:
    static ITweakPositionTextBox::Ptr create(ITweakPositionTextBox::Ptr child, const Point<double>& aspectRatio);
    [[nodiscard]] Point<double>
      parse(std::string_view x, std::string_view y, const Point<int>& viewSize, int zoom) const override;
    [[nodiscard]] std::pair<std::string, std::string>
      display(const Point<double>& position, const Point<int>& viewSize, int zoom) const override;

  private:
    ParseAndFormatViewCentreWithAspectRatio(ITweakPositionTextBox::Ptr child, const Point<double>& aspectRatio);
  };

} // namespace Scroom::Utils