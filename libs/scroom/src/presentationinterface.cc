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

#include <scroom/presentationinterface.hh>

#include <boost/foreach.hpp>

void PresentationBase::open(ViewInterface::WeakPtr vi)
{
  viewAdded(vi);

  std::list<Viewable::Ptr> const observers = getObservers();

  BOOST_FOREACH(Viewable::Ptr const& observer, observers)
  {
    observer->open(vi);
  }
}

void PresentationBase::close(ViewInterface::WeakPtr vi)
{
  std::list<Viewable::Ptr> const observers = getObservers();

  BOOST_FOREACH(Viewable::Ptr const& observer, observers)
  {
    observer->close(vi);
  }

  viewRemoved(vi);
}

void PresentationBase::observerAdded(Viewable::Ptr const& viewable, Scroom::Bookkeeping::Token const&)
{
  std::set<ViewInterface::WeakPtr> views = getViews();

  BOOST_FOREACH(ViewInterface::WeakPtr const& view, views)
  {
    viewable->open(view);
  }
}
