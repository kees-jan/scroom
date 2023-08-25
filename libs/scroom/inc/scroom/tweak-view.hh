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

    [[nodiscard]] virtual Point<double>
      tweakPosition(const Point<double>& currentPosition, const Point<int>& drawingAreaSize, int zoom) const = 0;
  };

  class ITweakRulers : public Interface
  {
  public:
    using Ptr = std::shared_ptr<ITweakRulers>;

    [[nodiscard]] virtual Rectangle<double>
      tweakRulers(const Point<double>& currentPosition, const Point<int>& drawingAreaSize, int zoom) const = 0;
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
      parse(std::string_view x, std::string_view y, const Point<int>& drawingAreaSize, int zoom) const = 0;

    [[nodiscard]] virtual std::pair<std::string, std::string>
      display(const Point<double>& position, const Point<int>& drawingAreaSize, int zoom) const = 0;
  };

  ITweakPresentationPosition::Ptr getDefaultTweakPresentationPosition();
  ITweakRulers::Ptr getDefaultTweakRulers();
  ITweakSelection::Ptr getDefaultTweakSelection();
  ITweakSelection::Map getDefaultTweakSelectionMap();
  ITweakPositionTextBox::Ptr getDefaultTweakPositionTextBox();

  ////////////////////////////////////////////////////////////////////////

  class TweakPresentationPosition : public ITweakPresentationPosition
  {
  public:
    using Ptr = std::shared_ptr<TweakPresentationPosition>;

    static ITweakPresentationPosition::Ptr create(const Point<double>& aspectRatio_ = {1.0, 1.0});

    [[nodiscard]] Point<double>
      tweakPosition(const Point<double>& currentPosition, const Point<int>& drawingAreaSize, int zoom) const override;

  private:
    explicit TweakPresentationPosition(const Point<double>& aspectRatio_);

  private:
    Point<double> aspectRatio;
  };

  class TweakRulers : public ITweakRulers
  {
  public:
    using Ptr = std::shared_ptr<TweakRulers>;

    static ITweakRulers::Ptr create(const Point<double>& aspectRatio_ = {1.0, 1.0});

    [[nodiscard]] Rectangle<double>
      tweakRulers(const Point<double>& currentPosition, const Point<int>& drawingAreaSize, int zoom) const override;

  private:
    explicit TweakRulers(const Point<double>& aspectRatio_);

  private:
    Point<double> aspectRatio;
  };

  class TweakSelection : public ITweakSelection
  {
  public:
    using Ptr = std::shared_ptr<TweakSelection>;

    [[nodiscard]] virtual Rectangle<double> tweakSelection(const Rectangle<double>& selection) const = 0;

    [[nodiscard]] Selection tweakSelection(const Selection& selection) const override;

  protected:
    explicit TweakSelection(const Point<double>& aspectRatio_);

  protected:
    Point<double> aspectRatio;
  };

  class TweakGridSelection : public TweakSelection
  {
  public:
    static ITweakSelection::Ptr create(const Point<double>& aspectRatio_ = {1.0, 1.0});

    [[nodiscard]] Rectangle<double> tweakSelection(const Rectangle<double>& selection) const override;

    using TweakSelection::tweakSelection;
    using TweakSelection::TweakSelection;
  };

  class TweakPixelSelection : public TweakSelection
  {
  public:
    static ITweakSelection::Ptr create(const Point<double>& aspectRatio_ = {1.0, 1.0});

    [[nodiscard]] Rectangle<double> tweakSelection(const Rectangle<double>& selection) const override;

    using TweakSelection::tweakSelection;
    using TweakSelection::TweakSelection;
  };

  class TweakPositionTextBox : public ITweakPositionTextBox
  {
  public:
    using Ptr = std::shared_ptr<TweakPositionTextBox>;

    static ITweakPositionTextBox::Ptr create(const Point<double>& aspectRatio_ = {1.0, 1.0});

    [[nodiscard]] Point<double>
      parse(std::string_view x, std::string_view y, const Point<int>& drawingAreaSize, int zoom) const override;

    [[nodiscard]] std::pair<std::string, std::string>
      display(const Point<double>& position, const Point<int>& drawingAreaSize, int zoom) const override;

  private:
    explicit TweakPositionTextBox(const Point<double>& aspectRatio_);

  private:
    Point<double> aspectRatio;
  };

} // namespace Scroom::Utils