/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <fmt/format.h>

#include <boost/lexical_cast.hpp>

#include <scroom/cairo-helpers.hh>
#include <scroom/tweak-view.hh>

namespace Scroom::Utils
{
  class DefaultTweakPresentationPosition : public ITweakPresentationPosition
  {
  public:
    [[nodiscard]] Point<double>
      tweakPosition(Point<double> currentPosition, Point<int> /*drawingAreaSize*/, int /*zoom*/) const override
    {
      return currentPosition;
    }
  };

  class DefaultTweakSelection : public ITweakSelection
  {
  public:
    [[nodiscard]] Selection tweakSelection(Selection selection) const override { return selection; }
  };

  class TweakRulers : public Scroom::Utils::ITweakRulers
  {
  public:
    explicit TweakRulers(Point<double> aspectRatio_)
      : aspectRatio(aspectRatio_)
    {
    }

    TweakRulers()
      : TweakRulers({1, 1})
    {
    }

    [[nodiscard]] Scroom::Utils::Rectangle<double>
      tweakRulers(Point<double> currentPosition, Point<int> drawingAreaSize, int zoom) const override
    {
      return Scroom::Utils::make_rect(currentPosition, drawingAreaSize.to<double>() / pixelSizeFromZoom(zoom)) / aspectRatio;
    }

  private:
    Point<double> aspectRatio;
  };

  class TweakPositionTextBox
  {
  public:
    using Ptr   = std::shared_ptr<TweakPositionTextBox>;
    using Point = Scroom::Utils::Point<double>;

    static Ptr create(Point aspectRatio_) { return Ptr(new TweakPositionTextBox(aspectRatio_)); }

    [[nodiscard]] Point parse(std::string_view x, std::string_view y, Scroom::Utils::Point<int> drawingAreaSize, int zoom) const
    {
      const Point entered_position(boost::lexical_cast<double>(x), boost::lexical_cast<double>(y));

      return entered_position * aspectRatio - drawingAreaSize.to<double>() / pixelSizeFromZoom(zoom) / 2;
    }

    [[nodiscard]] std::pair<std::string, std::string>
      display(Point position, Scroom::Utils::Point<int> drawingAreaSize, int zoom) const
    {
      const Point center = (position + drawingAreaSize.to<double>() / pixelSizeFromZoom(zoom) / 2) / aspectRatio;

      return std::make_pair(fmt::format("{:.0f}", center.x), fmt::format("{:.0f}", center.y));
    }

    void setAspectRatio(Point aspectRatio_) { aspectRatio = aspectRatio_; }

  private:
    explicit TweakPositionTextBox(Point aspectRatio_)
      : aspectRatio(aspectRatio_)
    {
    }

  private:
    Point aspectRatio;
  };


  ITweakPresentationPosition::Ptr getDefaultTweakPresentationPosition()
  {
    return std::make_shared<DefaultTweakPresentationPosition>();
  }
  ITweakRulers::Ptr          getDefaultTweakRulers() { return std::make_shared<TweakRulers>(); }
  ITweakSelection::Ptr       getDefaultTweakSelection() { return std::make_shared<DefaultTweakSelection>(); }
  ITweakPositionTextBox::Ptr getDefaultTweakPositionTextBox() { return ITweakPositionTextBox::Ptr(); }

} // namespace Scroom::Utils
