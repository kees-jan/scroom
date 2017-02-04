/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "transparentoverlaypresentation.hh"

#include <math.h>

#include <sstream>

#include <boost/foreach.hpp>
#include <boost/assign.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <scroom/unused.hh>
#include <scroom/colormappable.hh>

namespace
{
  const std::list<Color> colors = boost::assign::list_of<Color>
    (Color(doubleFromByte(2), doubleFromByte(63), doubleFromByte(165)))
    (Color(doubleFromByte(142), doubleFromByte(6), doubleFromByte(59)))
    (Color(doubleFromByte(74), doubleFromByte(111), doubleFromByte(227)))
    (Color(doubleFromByte(211), doubleFromByte(63), doubleFromByte(106)))
    (Color(doubleFromByte(17), doubleFromByte(198), doubleFromByte(56)))
    (Color(doubleFromByte(239), doubleFromByte(151), doubleFromByte(8)))
    (Color(doubleFromByte(15), doubleFromByte(207), doubleFromByte(192)))
    (Color(doubleFromByte(247), doubleFromByte(156), doubleFromByte(212)));

  struct ColorComparer
  {
    bool operator()(Color const& left, Color const& right)
    {
      return
        boost::make_tuple(left.alpha, left.red, left.green, left.blue) <
        boost::make_tuple(right.alpha, right.red, right.green, right.blue);
    }
  };
}

TransparentOverlayPresentation::Ptr TransparentOverlayPresentation::create()
{
  return Ptr(new TransparentOverlayPresentation());
}

TransparentOverlayPresentation::TransparentOverlayPresentation()
  : sizeDeterminer(SizeDeterminer::create())
{
}

void TransparentOverlayPresentation::addPresentation(PresentationInterface::Ptr const& p)
{
  if(p)
  {
    if(p->isPropertyDefined(MONOCHROME_COLORMAPPABLE_PROPERTY_NAME))
    {
      setOptimalColor(p);
    }
    
    children.push_back(p);
    sizeDeterminer->add(p);

    BOOST_FOREACH(ViewDataMap::value_type const& v, viewData)
      v.second->addChild(p);

  }
  else
    printf("PANIC: Can't add a nonexistent presentation\n");
}

void TransparentOverlayPresentation::setOptimalColor(PresentationInterface::Ptr const& p)
{
  Colormappable::Ptr c = boost::dynamic_pointer_cast<Colormappable>(p);
  if(c)
  {
    std::map<Color, int, ColorComparer> currentColors;
    BOOST_FOREACH(PresentationInterface::Ptr const& child, children)
    {
      Colormappable::Ptr cChild = boost::dynamic_pointer_cast<Colormappable>(child);
      if(cChild && child->isPropertyDefined(MONOCHROME_COLORMAPPABLE_PROPERTY_NAME))
      {
        currentColors[cChild->getMonochromeColor()]++;
      }
    }

    Color minimumColor=c->getMonochromeColor();
    int minimumColorValue = currentColors[minimumColor];

    BOOST_FOREACH(Color const& color, colors)
    {
      if(currentColors[color] < minimumColorValue)
      {
        minimumColor = color;
        minimumColorValue = currentColors[color];
      }
    }

    c->setMonochromeColor(minimumColor);
  }
}

GdkRectangle TransparentOverlayPresentation::getRect()
{
  return sizeDeterminer->getRect();
}

void TransparentOverlayPresentation::viewAdded(ViewInterface::WeakPtr vi)
{
  TransparentOverlayViewInfo::Ptr tovi = TransparentOverlayViewInfo::create(vi, sizeDeterminer);
  tovi->addChildren(children);
  viewData[vi] = tovi;
}

void TransparentOverlayPresentation::viewRemoved(ViewInterface::WeakPtr vi)
{
  ViewDataMap::const_iterator e = viewData.find(vi);
  if(e != viewData.end())
  {
    e->second->close();
    viewData.erase(e->first);
  }
}

std::set<ViewInterface::WeakPtr> TransparentOverlayPresentation::getViews()
{
  std::set<ViewInterface::WeakPtr> result;
  for(auto const& p: viewData)
    result.insert(p.first);

  return result;
}

void TransparentOverlayPresentation::redraw(ViewInterface::Ptr const& vi, cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  ViewDataMap::const_iterator e = viewData.find(vi);
  if(e != viewData.end())
    e->second->redraw(cr, presentationArea, zoom);
}

bool TransparentOverlayPresentation::getProperty(const std::string& name, std::string& value)
{
  UNUSED(name);
  UNUSED(value);
  
  return false;
}
bool TransparentOverlayPresentation::isPropertyDefined(const std::string& name)
{
  UNUSED(name);
    
  return false;
}

std::string TransparentOverlayPresentation::getTitle()
{
  std::stringstream s;
  s << "Overlay(";
  bool hasPrevious=false;
  BOOST_FOREACH(PresentationInterface::Ptr const& child, children)
  {
    if(hasPrevious)
      s << ", ";
    s << child->getTitle();
    hasPrevious = true;
  }
  s << ")";

  return s.str();
}

