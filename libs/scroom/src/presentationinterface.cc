/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/presentationinterface.hh>
#include <scroom/viewinterface.hh>

void PresentationBase::open(ViewInterface::WeakPtr vi)
{
  viewAdded(vi);

  std::list<Viewable::Ptr> const observers = getObservers();

  for(Viewable::Ptr const& observer: observers)
  {
    observer->open(vi);
  }
}

void PresentationBase::close(ViewInterface::WeakPtr vi)
{
  std::list<Viewable::Ptr> const observers = getObservers();

  for(Viewable::Ptr const& observer: observers)
  {
    observer->close(vi);
  }

  viewRemoved(vi);
}

void PresentationBase::observerAdded(Viewable::Ptr const& viewable, Scroom::Bookkeeping::Token const&)
{
  std::set<ViewInterface::WeakPtr> views = getViews();

  for(ViewInterface::WeakPtr const& view: views)
  {
    viewable->open(view);
  }
}
