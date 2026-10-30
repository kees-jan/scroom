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
      [[nodiscard]] Point<double> tweakPosition(const Point<double>& currentPosition, int /*zoom*/) const override
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

  ITweakRulers::Ptr CalculateViewAreaForRulers::create() { return Ptr(new CalculateViewAreaForRulers()); }

  Rectangle<double>
    CalculateViewAreaForRulers::tweakRulers(const Point<double>& currentPosition, const Point<int>& viewSize, int zoom) const
  {
    return make_rect(currentPosition, viewSize.to<double>() / pixelSizeFromZoom(zoom));
  }

  ////////////////////////////////////////////////////////////////////////

  Selection TweakSelectionsAsRectangles::tweakSelection(const Selection& selection) const
  {
    const auto selectionArea = toRectangle(selection);
    const auto tweakedArea = tweakSelection(selectionArea);

    const auto startCorner = find_closest_corner(selection.start, selectionArea);
    const auto endCorner = find_opposed_corner(startCorner);

    const auto start = corner_getter(startCorner)(tweakedArea);
    const auto end = corner_getter(endCorner)(tweakedArea);

    return {start, end};
  }

  ////////////////////////////////////////////////////////////////////////

  ITweakPositionTextBox::Ptr ParseAndFormatViewCentre::create() { return Ptr(new ParseAndFormatViewCentre()); }

  Point<double>
    ParseAndFormatViewCentre::parse(std::string_view x, std::string_view y, const Point<int>& viewSize, int zoom) const
  {
    const Point<double> enteredCentre(boost::lexical_cast<double>(x), boost::lexical_cast<double>(y));

    return enteredCentre - viewSize.to<double>() / pixelSizeFromZoom(zoom) / 2;
  }

  std::pair<std::string, std::string>
    ParseAndFormatViewCentre::display(const Point<double>& position, const Point<int>& viewSize, int zoom) const
  {
    const Point<double> viewCentre = position + viewSize.to<double>() / pixelSizeFromZoom(zoom) / 2;

    return std::make_pair(fmt::format("{:.0f}", viewCentre.x), fmt::format("{:.0f}", viewCentre.y));
  }

  ////////////////////////////////////////////////////////////////////////

  ITweakPresentationPosition::Ptr getDefaultTweakPresentationPosition()
  {
    return std::make_shared<DefaultTweakPresentationPosition>();
  }
  ITweakRulers::Ptr getDefaultTweakRulers() { return CalculateViewAreaForRulers::create(); }
  ITweakSelection::Ptr getDefaultTweakSelection() { return std::make_shared<DefaultTweakSelection>(); }
  ITweakSelection::Map getDefaultTweakSelectionMap()
  {
    return ITweakSelection::Map{{SelectionType::DEFAULT, getDefaultTweakSelection()}};
  }
  ITweakPositionTextBox::Ptr getDefaultTweakPositionTextBox() { return ParseAndFormatViewCentre::create(); }

  TweakPresentationPositionsWithAspectRatio::TweakPresentationPositionsWithAspectRatio(
    ITweakPresentationPosition::Ptr child,
    const Point<double>& aspectRatio
  )
    : m_child(std::move(child))
    , m_aspectRatio(aspectRatio)
  {
    require(m_child);
  }

  ITweakPresentationPosition::Ptr
    TweakPresentationPositionsWithAspectRatio::create(ITweakPresentationPosition::Ptr child, const Point<double>& aspectRatio)
  {
    return ITweakPresentationPosition::Ptr(new TweakPresentationPositionsWithAspectRatio(std::move(child), aspectRatio));
  }

  Point<double> TweakPresentationPositionsWithAspectRatio::tweakPosition(const Point<double>& currentPosition, int zoom) const
  {
    return m_child->tweakPosition(currentPosition / m_aspectRatio, zoom) * m_aspectRatio;
  }

  TweakRulersWithAspectRatio::TweakRulersWithAspectRatio(ITweakRulers::Ptr child, const Point<double>& aspectRatio)
    : m_child(std::move(child))
    , m_aspectRatio(aspectRatio)
  {
    require(m_child);
  }

  ITweakRulers::Ptr TweakRulersWithAspectRatio::create(ITweakRulers::Ptr child, const Point<double>& aspectRatio)
  {
    return ITweakRulers::Ptr(new TweakRulersWithAspectRatio(std::move(child), aspectRatio));
  }

  Rectangle<double>
    TweakRulersWithAspectRatio::tweakRulers(const Point<double>& currentPosition, const Point<int>& viewSize, int zoom) const
  {
    return m_child->tweakRulers(currentPosition, viewSize, zoom) / m_aspectRatio;
  }

  TweakSelectionsWithAspectRatio::TweakSelectionsWithAspectRatio(ITweakSelection::Ptr child, const Point<double>& aspectRatio)
    : m_child(std::move(child))
    , m_aspectRatio(aspectRatio)
  {
    require(m_child);
  }

  ITweakSelection::Ptr TweakSelectionsWithAspectRatio::create(ITweakSelection::Ptr child, const Point<double>& aspectRatio)
  {
    return ITweakSelection::Ptr(new TweakSelectionsWithAspectRatio(std::move(child), aspectRatio));
  }

  Selection TweakSelectionsWithAspectRatio::tweakSelection(const Selection& selection) const
  {
    const auto childSelection =
      m_child->tweakSelection(Selection{selection.start / m_aspectRatio, selection.end / m_aspectRatio});
    return {childSelection.start * m_aspectRatio, childSelection.end * m_aspectRatio};
  }

  ParseAndFormatViewCentreWithAspectRatio::ParseAndFormatViewCentreWithAspectRatio(
    ITweakPositionTextBox::Ptr child,
    const Point<double>& aspectRatio
  )
    : m_child(std::move(child))
    , m_aspectRatio(aspectRatio)
  {
    require(m_child);
  }

  ITweakPositionTextBox::Ptr
    ParseAndFormatViewCentreWithAspectRatio::create(ITweakPositionTextBox::Ptr child, const Point<double>& aspectRatio)
  {
    return ITweakPositionTextBox::Ptr(new ParseAndFormatViewCentreWithAspectRatio(std::move(child), aspectRatio));
  }

  Point<double> ParseAndFormatViewCentreWithAspectRatio::parse(
    std::string_view x,
    std::string_view y,
    const Point<int>& viewSize,
    int zoom
  ) const
  {
    // Textboxes describe the view centre, whereas parse returns its origin.
    const auto halfViewSizeInPresentationCoordinates = viewSize.to<double>() / pixelSizeFromZoom(zoom) / 2;
    return (m_child->parse(x, y, viewSize, zoom) + halfViewSizeInPresentationCoordinates) * m_aspectRatio
           - halfViewSizeInPresentationCoordinates;
  }

  std::pair<std::string, std::string>
    ParseAndFormatViewCentreWithAspectRatio::display(const Point<double>& position, const Point<int>& viewSize, int zoom) const
  {
    const auto halfViewSizeInPresentationCoordinates = viewSize.to<double>() / pixelSizeFromZoom(zoom) / 2;
    return m_child->display(
      (position + halfViewSizeInPresentationCoordinates) / m_aspectRatio - halfViewSizeInPresentationCoordinates, viewSize, zoom
    );
  }

} // namespace Scroom::Utils
