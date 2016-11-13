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

#include "sizedeterminer.hh"

#include <algorithm>

#include <boost/foreach.hpp>

#include <scroom/gtk-helpers.hh>
#include <scroom/assertions.hh>

namespace
{
  GdkRectangle DetermineSize(std::list<PresentationInterface::Ptr> presentations)
  {
    int left = std::numeric_limits<int>::max();
    int top = std::numeric_limits<int>::max();
    int right = std::numeric_limits<int>::min();
    int bottom = std::numeric_limits<int>::min();

    BOOST_FOREACH(PresentationInterface::Ptr const& p, presentations)
    {
      GdkRectangle rect = p->getRect();
      left = std::min(left, rect.x);
      top = std::min(top, rect.y);
      right = std::max(right, rect.x + rect.width);
      bottom = std::max(bottom, rect.y + rect.height);
    }
    return Scroom::GtkHelpers::createGdkRectangle(left, top, right-left, bottom-top);
  }
}

SizeDeterminer::Ptr SizeDeterminer::create()
{
  return Ptr(new SizeDeterminer());
}

SizeDeterminer::SizeDeterminer()
{
}

void SizeDeterminer::add(PresentationInterface::Ptr const& p)
{
  ResizablePresentationInterface::Ptr r = boost::dynamic_pointer_cast<ResizablePresentationInterface>(p);
  if(r)
  {
    resizablePresentations.push_back(p);
    resizablePresentationInterfaces.push_back(r);
  }
  else
    presentations.push_back(p);

  sendUpdates();
}

GdkRectangle SizeDeterminer::getRect() const
{
  if(!presentations.empty())
  {
    return DetermineSize(presentations);
  }
  if(!resizablePresentations.empty())
  {
    return DetermineSize(resizablePresentations);
  }
  return Scroom::GtkHelpers::createGdkRectangle(0,0,0,0);
}

void SizeDeterminer::open(ViewInterface::WeakPtr vi)
{
  require(!views.count(vi));
  views.insert(vi);

  sendUpdates(vi, getRect());
}

void SizeDeterminer::close(ViewInterface::WeakPtr vi)
{
  require(views.count(vi));
  views.erase(vi);
}

void SizeDeterminer::sendUpdates(ViewInterface::WeakPtr const& vi, GdkRectangle const& rect)
{
  BOOST_FOREACH(ResizablePresentationInterface::Ptr const& r, resizablePresentationInterfaces)
  {
    r->setRect(vi, rect);
  }
}

void SizeDeterminer::sendUpdates()
{
  GdkRectangle rect = getRect();

  BOOST_FOREACH(ViewInterface::WeakPtr const& vi, views)
  {
    sendUpdates(vi, rect);
  }
}
