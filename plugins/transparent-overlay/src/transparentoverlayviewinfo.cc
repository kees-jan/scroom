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

#include "transparentoverlayviewinfo.hh"

#include <boost/foreach.hpp>

#include <scroom/unused.hh>

////////////////////////////////////////////////////////////////////////
// ChildView
////////////////////////////////////////////////////////////////////////

ChildView::ChildView(TransparentOverlayViewInfo::Ptr const& parent)
  : parent(parent), progressInterface(parent->getProgressInterface())
{}

ChildView::Ptr ChildView::create(TransparentOverlayViewInfo::Ptr const& parent)
{
  return Ptr(new ChildView(parent));
}

void ChildView::invalidate()
{
  parent->invalidate();
}

ProgressInterface::Ptr ChildView::getProgressInterface()
{
  return progressInterface;
}

void ChildView::addSideWidget(std::string, GtkWidget*)
{}

void ChildView::removeSideWidget(GtkWidget*)
{}

void ChildView::addToToolbar(GtkToolItem*)
{}

void ChildView::removeFromToolbar(GtkToolItem*)
{}

////////////////////////////////////////////////////////////////////////
// TransparentOverlayViewInfo
////////////////////////////////////////////////////////////////////////

TransparentOverlayViewInfo::TransparentOverlayViewInfo(const ViewInterface::WeakPtr& vi)
  : parentView(vi),
    progressInterfaceMultiplexer(Scroom::Utils::ProgressInterfaceMultiplexer::create(parentView->getProgressInterface()))
{}

TransparentOverlayViewInfo::Ptr TransparentOverlayViewInfo::create(const ViewInterface::WeakPtr& vi)
{
  return Ptr(new TransparentOverlayViewInfo(vi));
}

void TransparentOverlayViewInfo::addChildren(const std::list<PresentationInterface::Ptr>& children)
{
  BOOST_FOREACH(PresentationInterface::Ptr const& child, children)
    addChild(child);
}

void TransparentOverlayViewInfo::addChild(const PresentationInterface::Ptr& child)
{
  ChildView::Ptr view = ChildView::create(shared_from_this<TransparentOverlayViewInfo>());
  children[child] = view;
  child->open(view);
}

void TransparentOverlayViewInfo::close()
{
  BOOST_FOREACH(ChildMap::value_type const& v, children)
    v.first->close(v.second);

  children.clear();
}

void TransparentOverlayViewInfo::redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  BOOST_FOREACH(ChildMap::value_type const& v, children)
    v.first->redraw(v.second, cr, presentationArea, zoom);
}
