/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

#include <scroom/unused.hh>

TransparentOverlayPresentation::Ptr TransparentOverlayPresentation::create()
{
  return Ptr(new TransparentOverlayPresentation());
}

TransparentOverlayPresentation::TransparentOverlayPresentation()
{
}

void TransparentOverlayPresentation::addPresentation(PresentationInterface::Ptr const& p)
{
  children.push_back(p);
  if(!favorite)
    favorite=p;

  BOOST_FOREACH(ViewDataMap::value_type const& v, viewData)
    v.second->addChild(p);
}

GdkRectangle TransparentOverlayPresentation::getRect()
{
  GdkRectangle rect;

  if(favorite)
    rect = favorite->getRect();
  else
  {
    rect.x=-500;
    rect.y=-500;
    rect.width=1000;
    rect.height=1000;
  }
  
  return rect;
}

void TransparentOverlayPresentation::open(ViewInterface::WeakPtr vi)
{
  TransparentOverlayViewInfo::Ptr tovi = TransparentOverlayViewInfo::create(vi);
  tovi->addChildren(children);
  viewData[vi] = tovi;
}

void TransparentOverlayPresentation::close(ViewInterface::WeakPtr vi)
{
  ViewDataMap::const_iterator e = viewData.find(vi);
  if(e != viewData.end())
  {
    e->second->close();
    viewData.erase(e);
  }
}

void TransparentOverlayPresentation::redraw(ViewInterface::Ptr vi, cairo_t* cr, GdkRectangle presentationArea, int zoom)
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

