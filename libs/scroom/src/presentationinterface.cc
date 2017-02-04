/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
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
