/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "transparentoverlayviewinfo.hh"

#include <sstream>

#include <scroom/bitmap-helpers.hh>
#include <scroom/colormappable.hh>
#include <scroom/unused.hh>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

////////////////////////////////////////////////////////////////////////

namespace
{
  void setToggleButtonColor(GtkWidget* w, PresentationInterface::Ptr const& p)
  {
    Colormappable::Ptr c = boost::dynamic_pointer_cast<Colormappable>(p);
    if(c && p->isPropertyDefined(MONOCHROME_COLORMAPPABLE_PROPERTY_NAME))
    {
      Color           col           = c->getMonochromeColor();
      GtkCssProvider* bgCssProvider = gtk_css_provider_new();
      std::string     bgCss         = "* { background-image:none; background-color: #" + col.getHex() + ";}";

      // Fill the provider with the correct data bgCss string
      gtk_css_provider_load_from_data(bgCssProvider, bgCss.c_str(), -1, nullptr);

      // Get the style context
      GtkStyleContext* bgContext = gtk_widget_get_style_context(w);

      // Add the provider
      gtk_style_context_add_provider(bgContext, GTK_STYLE_PROVIDER(bgCssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);


      std::string     fgCss         = "* { color: #" + col.getContrastingBlackOrWhite().getHex() + ";}";
      GtkWidget*      label         = gtk_bin_get_child(GTK_BIN(w));
      GtkCssProvider* fgCssProvider = gtk_css_provider_new();

      // Fill the provider with the correct data fgCss string
      gtk_css_provider_load_from_data(fgCssProvider, fgCss.c_str(), -1, nullptr);

      // Get the style context
      GtkStyleContext* fgContext = gtk_widget_get_style_context(label);

      // Add the provider
      gtk_style_context_add_provider(fgContext, GTK_STYLE_PROVIDER(fgCssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
  }
} // namespace

////////////////////////////////////////////////////////////////////////
// ChildView
////////////////////////////////////////////////////////////////////////

ChildView::ChildView(TransparentOverlayViewInfo::Ptr const& parent_)
  : parent(parent_)
  , progressInterface(parent->getProgressInterface())
{
}

ChildView::Ptr ChildView::create(TransparentOverlayViewInfo::Ptr const& parent) { return Ptr(new ChildView(parent)); }

void ChildView::invalidate() { parent->invalidate(); }

ProgressInterface::Ptr ChildView::getProgressInterface() { return progressInterface; }

void ChildView::addSideWidget(std::string, GtkWidget*) {}

void ChildView::removeSideWidget(GtkWidget*) {}

void ChildView::addToToolbar(GtkToolItem*) {}

void ChildView::removeFromToolbar(GtkToolItem*) {}

void ChildView::registerSelectionListener(SelectionListener::Ptr) {}

void ChildView::registerPostRenderer(PostRenderer::Ptr) {}

void ChildView::setStatusMessage(const std::string&) {}

PresentationInterface::Ptr ChildView::getCurrentPresentation() { return parent->getChild(shared_from_this<ChildView>()); }

void ChildView::addToolButton(GtkToggleButton*, ToolStateListener::Ptr) {}

////////////////////////////////////////////////////////////////////////
// TransparentOverlayViewInfo
////////////////////////////////////////////////////////////////////////

TransparentOverlayViewInfo::TransparentOverlayViewInfo(const ViewInterface::WeakPtr& vi,
                                                       SizeDeterminer::Ptr const&    sizeDeterminer_)
  : parentView(vi)
  , progressInterfaceMultiplexer(Scroom::Utils::ProgressInterfaceMultiplexer::create(parentView->getProgressInterface()))
  , sizeDeterminer(sizeDeterminer_)
{
}

static void on_toggled(GtkToggleButton* button, gpointer data)
{
  static_cast<TransparentOverlayViewInfo*>(data)->toggled(button);
}

void TransparentOverlayViewInfo::createToggleToolButton(PresentationInterface::Ptr const& p)
{
  int               n = buttons.size() + 1;
  std::stringstream s;
  s << "_" << n;

  GtkToolItem* button       = gtk_tool_item_new();
  GtkWidget*   toggleButton = gtk_toggle_button_new_with_mnemonic(s.str().c_str());
  setToggleButtonColor(toggleButton, p);
  gtk_widget_set_visible(toggleButton, true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggleButton), true);

  gtk_container_add(GTK_CONTAINER(button), toggleButton);
  g_signal_connect(static_cast<gpointer>(toggleButton), "toggled", G_CALLBACK(on_toggled), this);

  buttons.push_back(toggleButton);

  parentView->addToToolbar(button);
}

TransparentOverlayViewInfo::Ptr TransparentOverlayViewInfo::create(const ViewInterface::WeakPtr& vi,
                                                                   SizeDeterminer::Ptr const&    sizeDeterminer)
{
  return Ptr(new TransparentOverlayViewInfo(vi, sizeDeterminer));
}

void TransparentOverlayViewInfo::addChildren(const std::list<PresentationInterface::Ptr>& children_)
{
  for(PresentationInterface::Ptr const& child: children_)
  {
    addChild(child);
  }
}

void TransparentOverlayViewInfo::addChild(const PresentationInterface::Ptr& child)
{
  ChildView::Ptr view = ChildView::create(shared_from_this<TransparentOverlayViewInfo>());
  childViews[child]   = view;
  child->open(view);
  sizeDeterminer->open(child, view);
  children.push_back(child);
  createToggleToolButton(child);
}

PresentationInterface::Ptr TransparentOverlayViewInfo::getChild(const ChildView::Ptr& cv)
{
  for(ChildMap::value_type const& v: childViews)
  {
    if(v.second == cv)
    {
      return v.first;
    }
  }
  return PresentationInterface::Ptr();
}

void TransparentOverlayViewInfo::close()
{
  for(ChildMap::value_type const& v: childViews)
  {
    sizeDeterminer->close(v.first, v.second);
    v.first->close(v.second);
  }

  childViews.clear();
  children.clear();
  buttons.clear();
}

void TransparentOverlayViewInfo::redraw(cairo_t* cr, Scroom::Utils::Rectangle<double> const& pa, int zoom)
{
  using Scroom::Bitmap::BitmapSurface;

  cairo_rectangle_int_t presentationArea = pa.toGdkRectangle();

  cairo_rectangle_int_t viewArea;
  viewArea.x = 0;
  viewArea.y = 0;

  if(zoom > 0)
  {
    const int pixelSize = 1 << zoom;
    viewArea.width      = presentationArea.width * pixelSize;
    viewArea.height     = presentationArea.height * pixelSize;
  }
  else
  {
    const int pixelSize = 1 << -zoom;
    viewArea.width      = presentationArea.width / pixelSize;
    viewArea.height     = presentationArea.height / pixelSize;
  }

  BitmapSurface::Ptr s       = BitmapSurface::create(viewArea.width, viewArea.height, CAIRO_FORMAT_ARGB32);
  cairo_surface_t*   surface = s->get();
  cairo_t*           cr_sub  = cairo_create(surface);

  int count = 0;

  for(size_t i = 0; i < children.size(); i++)
  {
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttons[i])))
    {
      PresentationInterface::Ptr& p = children[i];

      Colormappable::Ptr c                        = boost::dynamic_pointer_cast<Colormappable>(p);
      bool               hasTransparentBackground = false;
      if(c && p->isPropertyDefined(TRANSPARENT_BACKGROUND_PROPERTY_NAME))
      {
        if(count == 0)
        {
          c->disableTransparentBackground();
        }
        else
        {
          c->setTransparentBackground();
          hasTransparentBackground = true;
        }
      }

      cairo_save(cr_sub);
      cairo_set_operator(cr_sub, CAIRO_OPERATOR_CLEAR);
      cairo_paint(cr_sub);
      cairo_restore(cr_sub);

      cairo_save(cr_sub);
      p->redraw(childViews[p], cr_sub, pa, zoom);
      cairo_restore(cr_sub);

      cairo_set_source_surface(cr, surface, 0, 0);

      if(hasTransparentBackground)
      {
        cairo_paint(cr);
      }
      else
      {
        cairo_paint_with_alpha(cr, 1.0 / (count + 1));
      }

      if(!hasTransparentBackground)
      {
        count++;
      }
    }
  }

  cairo_destroy(cr_sub);
}

void TransparentOverlayViewInfo::toggled(GtkToggleButton*) { invalidate(); }
