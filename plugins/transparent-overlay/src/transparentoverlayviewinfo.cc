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

#include "transparentoverlayviewinfo.hh"

#include <sstream>

#include <boost/foreach.hpp>

#include <scroom/unused.hh>
#include <scroom/bitmap-helpers.hh>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

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

TransparentOverlayViewInfo::TransparentOverlayViewInfo(const ViewInterface::WeakPtr& vi, SizeDeterminer::Ptr const& sizeDeterminer)
  : parentView(vi),
    progressInterfaceMultiplexer(Scroom::Utils::ProgressInterfaceMultiplexer::create(parentView->getProgressInterface())),
    sizeDeterminer(sizeDeterminer)
{}

static void on_toggled(GtkToggleButton* button, gpointer data)
{
  ((TransparentOverlayViewInfo*)data)->toggled(button);
}

void TransparentOverlayViewInfo::createToggleToolButton()
{
  int n = buttons.size()+1;
  std::stringstream s;
  s << "_" << n;
  
  GtkToolItem* button = gtk_tool_item_new();
  GtkWidget* toggleButton = gtk_toggle_button_new_with_mnemonic(s.str().c_str());
  gtk_widget_set_visible(toggleButton, true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggleButton), true);
  
  gtk_container_add(GTK_CONTAINER(button), toggleButton);
  g_signal_connect((gpointer)toggleButton, "toggled", G_CALLBACK(on_toggled), this);

  buttons.push_back(toggleButton);
  
  parentView->addToToolbar(button);
}

TransparentOverlayViewInfo::Ptr TransparentOverlayViewInfo::create(const ViewInterface::WeakPtr& vi, SizeDeterminer::Ptr const& sizeDeterminer)
{
  return Ptr(new TransparentOverlayViewInfo(vi, sizeDeterminer));
}

void TransparentOverlayViewInfo::addChildren(const std::list<PresentationInterface::Ptr>& children)
{
  BOOST_FOREACH(PresentationInterface::Ptr const& child, children)
    addChild(child);
}

void TransparentOverlayViewInfo::addChild(const PresentationInterface::Ptr& child)
{
  ChildView::Ptr view = ChildView::create(shared_from_this<TransparentOverlayViewInfo>());
  childViews[child] = view;
  child->open(view);
  sizeDeterminer->open(child, view);
  children.push_back(child);
  createToggleToolButton();
}

void TransparentOverlayViewInfo::close()
{
  BOOST_FOREACH(ChildMap::value_type const& v, childViews)
  {
    sizeDeterminer->close(v.first, v.second);
    v.first->close(v.second);
  }
  
  childViews.clear();
  children.clear();
  buttons.clear();
}

void TransparentOverlayViewInfo::redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  using Scroom::Bitmap::BitmapSurface;
  
  GdkRectangle viewArea;
  viewArea.x=0;
  viewArea.y=0;
  
  if(zoom > 0)
  {
    const int pixelSize = 1<<zoom;
    viewArea.width = presentationArea.width*pixelSize;
    viewArea.height = presentationArea.height*pixelSize;
  }
  else
  {
    const int pixelSize = 1<<-zoom;
    viewArea.width = presentationArea.width/pixelSize;
    viewArea.height = presentationArea.height/pixelSize;
  }

  BitmapSurface::Ptr s = BitmapSurface::create(viewArea.width, viewArea.height);
  cairo_surface_t* surface = s->get();
  cairo_t* cr_sub = cairo_create(surface);

  int count=0;

  for(size_t i=0; i<children.size(); i++)
  {
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttons[i])))
    {
      PresentationInterface::Ptr& p = children[i];
      cairo_save(cr_sub);
      p->redraw(childViews[p], cr_sub, presentationArea, zoom);
      cairo_restore(cr_sub);

      cairo_set_source_surface(cr, surface, 0, 0);
      cairo_paint_with_alpha(cr, 1.0/++count);
    }
  }

  cairo_destroy(cr_sub);
}

void TransparentOverlayViewInfo::toggled(GtkToggleButton*)
{
  invalidate();
}
