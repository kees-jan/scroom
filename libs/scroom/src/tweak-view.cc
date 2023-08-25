/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <array>
#include <functional>

#include <fmt/format.h>

#include <boost/lexical_cast.hpp>

#include <scroom/assertions.hh>
#include <scroom/cairo-helpers.hh>
#include <scroom/tweak-view.hh>

namespace Scroom::Utils
{
  namespace
  {
    class DefaultTweakPresentationPosition : public ITweakPresentationPosition
    {
    public:
      [[nodiscard]] Point<double>
        tweakPosition(const Point<double>& currentPosition, const Point<int> /*drawingAreaSize*/&, int /*zoom*/) const override
      {
        return currentPosition;
      }
    };

    class DefaultTweakSelection : public ITweakSelection
    {
    public:
      [[nodiscard]] Selection tweakSelection(const Selection& selection) const override { return selection; }
    };

    enum class Corner
    {
      TOP_LEFT,
      TOP_RIGHT,
      BOTTOM_LEFT,
      BOTTOM_RIGHT
    };

    std::function<Point<double>(const Rectangle<double>&)> corner_getter(Corner c)
    {
      switch(c)
      {
      case Corner::TOP_LEFT:
        return [](const Rectangle<double>& r) { return r.getTopLeft(); };
      case Corner::TOP_RIGHT:
        return [](const Rectangle<double>& r) { return r.getTopRight(); };
      case Corner::BOTTOM_LEFT:
        return [](const Rectangle<double>& r) { return r.getBottomLeft(); };
      case Corner::BOTTOM_RIGHT:
        return [](const Rectangle<double>& r) { return r.getBottomRight(); };
      }
      defect();
    }

    Corner find_closest_corner(const Point<double>& p, const Rectangle<double>& r)
    {
      constexpr std::array<std::array<Corner, 2>, 2> corners = {{
        {Corner::TOP_LEFT, Corner::BOTTOM_LEFT},
        {Corner::TOP_RIGHT, Corner::BOTTOM_RIGHT},
      }};

      const auto c = center(r);

      return corners.at(c.x < p.x).at(c.y < p.y);
    }

    Corner find_opposed_corner(Corner c)
    {
      switch(c)
      {
      case Corner::TOP_LEFT:
        return Corner::BOTTOM_RIGHT;
      case Corner::TOP_RIGHT:
        return Corner::BOTTOM_LEFT;
      case Corner::BOTTOM_LEFT:
        return Corner::TOP_RIGHT;
      case Corner::BOTTOM_RIGHT:
        return Corner::TOP_LEFT;
      }
      defect();
    }

    Rectangle<double> toRectangle(const Selection& s) { return make_rect_from_start_end(s.start, s.end); }
  } // namespace

  ////////////////////////////////////////////////////////////////////////

  TweakPresentationPosition::TweakPresentationPosition(const Point<double>& aspectRatio_)
    : aspectRatio(aspectRatio_)
  {
  }

  ITweakPresentationPosition::Ptr TweakPresentationPosition::create(const Point<double>& aspectRatio_)
  {
    return Ptr(new TweakPresentationPosition(aspectRatio_));
  }

  Point<double> TweakPresentationPosition::tweakPosition(
    const Point<double>& currentPosition,
    const Point<int> /*drawingAreaSize*/&,
    int zoom
  ) const
  {
    return round_to_multiple_of(currentPosition, aspectRatio / pixelSizeFromZoom(zoom));
  }

  ////////////////////////////////////////////////////////////////////////

  TweakRulers::TweakRulers(const Point<double>& aspectRatio_)
    : aspectRatio(aspectRatio_)
  {
  }

  ITweakRulers::Ptr TweakRulers::create(const Point<double>& aspectRatio_) { return Ptr(new TweakRulers(aspectRatio_)); }

  Rectangle<double>
    TweakRulers::tweakRulers(const Point<double>& currentPosition, const Point<int>& drawingAreaSize, int zoom) const
  {
    return make_rect(currentPosition, drawingAreaSize.to<double>() / pixelSizeFromZoom(zoom)) / aspectRatio;
  }

  ////////////////////////////////////////////////////////////////////////

  TweakSelection::TweakSelection(const Point<double>& aspectRatio_)
    : aspectRatio(aspectRatio_)
  {
  }

  Selection TweakSelection::tweakSelection(const Selection& selection) const
  {
    const auto original = toRectangle(selection);
    const auto tweaked = tweakSelection(original);

    const auto startCorner = find_closest_corner(selection.start, original);
    const auto endCorner = find_opposed_corner(startCorner);

    const auto start = corner_getter(startCorner)(tweaked);
    const auto end = corner_getter(endCorner)(tweaked);

    return {start, end};
  }

  ////////////////////////////////////////////////////////////////////////

  ITweakSelection::Ptr TweakGridSelection::create(const Point<double>& aspectRatio_)
  {
    return Ptr(new TweakGridSelection(aspectRatio_));
  }

  Rectangle<double> TweakGridSelection::tweakSelection(const Rectangle<double>& selection) const
  {
    return roundCorners(selection / aspectRatio) * aspectRatio;
  }

  ////////////////////////////////////////////////////////////////////////

  ITweakSelection::Ptr TweakPixelSelection::create(const Point<double>& aspectRatio_)
  {
    return Ptr(new TweakPixelSelection(aspectRatio_));
  }

  Rectangle<double> TweakPixelSelection::tweakSelection(const Rectangle<double>& selection) const
  {
    return roundOutward(selection / aspectRatio) * aspectRatio;
  }

  ////////////////////////////////////////////////////////////////////////

  TweakPositionTextBox::TweakPositionTextBox(const Point<double>& aspectRatio_)
    : aspectRatio(aspectRatio_)
  {
  }

  ITweakPositionTextBox::Ptr TweakPositionTextBox::create(const Point<double>& aspectRatio_)
  {
    return Ptr(new TweakPositionTextBox(aspectRatio_));
  }

  Point<double>
    TweakPositionTextBox::parse(std::string_view x, std::string_view y, const Point<int>& drawingAreaSize, int zoom) const
  {
    const Point<double> entered_position(boost::lexical_cast<double>(x), boost::lexical_cast<double>(y));

    return entered_position * aspectRatio - drawingAreaSize.to<double>() / pixelSizeFromZoom(zoom) / 2;
  }

  std::pair<std::string, std::string>
    TweakPositionTextBox::display(const Point<double>& position, const Point<int>& drawingAreaSize, int zoom) const
  {
    const Point<double> c = (position + drawingAreaSize.to<double>() / pixelSizeFromZoom(zoom) / 2) / aspectRatio;

    return std::make_pair(fmt::format("{:.0f}", c.x), fmt::format("{:.0f}", c.y));
  }

  ////////////////////////////////////////////////////////////////////////

  ITweakPresentationPosition::Ptr getDefaultTweakPresentationPosition()
  {
    return std::make_shared<DefaultTweakPresentationPosition>();
  }
  ITweakRulers::Ptr getDefaultTweakRulers() { return TweakRulers::create(); }
  ITweakSelection::Ptr getDefaultTweakSelection() { return std::make_shared<DefaultTweakSelection>(); }
  ITweakSelection::Map getDefaultTweakSelectionMap()
  {
    return TweakSelection::Map{{SelectionType::DEFAULT, getDefaultTweakSelection()}};
  }
  ITweakPositionTextBox::Ptr getDefaultTweakPositionTextBox() { return TweakPositionTextBox::create(); }

} // namespace Scroom::Utils
