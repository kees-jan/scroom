/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "transparentoverlaypresentation.hh"

#include <cmath>
#include <sstream>

#include <boost/assign.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <scroom/colormappable.hh>


namespace
{
  const std::list<Color> colors =
    boost::assign::list_of<Color>(Color(doubleFromByte(2), doubleFromByte(63), doubleFromByte(165)))(
      Color(doubleFromByte(142), doubleFromByte(6), doubleFromByte(59)))(
      Color(doubleFromByte(74), doubleFromByte(111), doubleFromByte(227)))(
      Color(doubleFromByte(211), doubleFromByte(63), doubleFromByte(106)))(
      Color(doubleFromByte(17), doubleFromByte(198), doubleFromByte(56)))(
      Color(doubleFromByte(239), doubleFromByte(151), doubleFromByte(8)))(
      Color(doubleFromByte(15), doubleFromByte(207), doubleFromByte(192)))(
      Color(doubleFromByte(247), doubleFromByte(156), doubleFromByte(212)));

  struct ColorComparer
  {
    bool operator()(Color const& left, Color const& right) const
    {
      return boost::make_tuple(left.alpha, left.red, left.green, left.blue)
             < boost::make_tuple(right.alpha, right.red, right.green, right.blue);
    }
  };
} // namespace

TransparentOverlayPresentation::Ptr TransparentOverlayPresentation::create() { return Ptr(new TransparentOverlayPresentation()); }

TransparentOverlayPresentation::TransparentOverlayPresentation()
  : sizeDeterminer(SizeDeterminer::create())
  , context(Scroom::Utils::RecursiveContext::Create())
{
}

void TransparentOverlayPresentation::addPresentation(PresentationInterface::Ptr const& p)
{
  if(p)
  {
    context->add(p->getContext());

    if(p->isPropertyDefined(MONOCHROME_COLORMAPPABLE_PROPERTY_NAME))
    {
      setOptimalColor(p);
    }

    children.push_back(p);
    sizeDeterminer->add(p);

    for(ViewDataMap::value_type const& v: viewData)
    {
      v.second->addChild(p);
    }
  }
  else
  {
    defect_message("Can't add a nonexistent presentation");
  }
}

void TransparentOverlayPresentation::setOptimalColor(PresentationInterface::Ptr const& p)
{
  Colormappable::Ptr const c = std::dynamic_pointer_cast<Colormappable>(p);
  if(c)
  {
    std::map<Color, int, ColorComparer> currentColors;
    for(PresentationInterface::Ptr const& child: children)
    {
      Colormappable::Ptr const cChild = std::dynamic_pointer_cast<Colormappable>(child);
      if(cChild && child->isPropertyDefined(MONOCHROME_COLORMAPPABLE_PROPERTY_NAME))
      {
        currentColors[cChild->getMonochromeColor()]++;
      }
    }

    Color minimumColor      = c->getMonochromeColor();
    int   minimumColorValue = currentColors[minimumColor];

    for(Color const& color: colors)
    {
      if(currentColors[color] < minimumColorValue)
      {
        minimumColor      = color;
        minimumColorValue = currentColors[color];
      }
    }

    c->setMonochromeColor(minimumColor);
  }
}

Scroom::Utils::Rectangle<double> TransparentOverlayPresentation::getRect() { return sizeDeterminer->getRect(); }

void TransparentOverlayPresentation::viewAdded(ViewInterface::WeakPtr vi)
{
  TransparentOverlayViewInfo::Ptr const tovi = TransparentOverlayViewInfo::create(vi, sizeDeterminer);
  tovi->addChildren(children);
  viewData[vi] = tovi;
}

void TransparentOverlayPresentation::viewRemoved(ViewInterface::WeakPtr vi)
{
  auto e = viewData.find(vi);
  if(e != viewData.end())
  {
    e->second->close();
    viewData.erase(e->first);
  }
}

Scroom::Utils::WeakKeySet<ViewInterface::WeakPtr> TransparentOverlayPresentation::getViews()
{
  Scroom::Utils::WeakKeySet<ViewInterface::WeakPtr> result;
  for(auto const& p: viewData)
  {
    result.insert(p.first);
  }

  return result;
}

void TransparentOverlayPresentation::redraw(ViewInterface::Ptr const&        vi,
                                            cairo_t*                         cr,
                                            Scroom::Utils::Rectangle<double> presentationArea,
                                            int                              zoom)
{
  auto e = viewData.find(vi);
  if(e != viewData.end())
  {
    e->second->redraw(cr, presentationArea, zoom);
  }
}

bool TransparentOverlayPresentation::getProperty(const std::string& /*name*/, std::string& /*value*/) { return false; }

bool TransparentOverlayPresentation::isPropertyDefined(const std::string& /*name*/) { return false; }

std::string TransparentOverlayPresentation::getTitle()
{
  std::stringstream s;
  s << "Overlay(";
  bool hasPrevious = false;
  for(PresentationInterface::Ptr const& child: children)
  {
    if(hasPrevious)
    {
      s << ", ";
    }
    s << child->getTitle();
    hasPrevious = true;
  }
  s << ")";

  return s.str();
}
Scroom::Utils::Context::ConstPtr TransparentOverlayPresentation::getContext() const { return context; }
