/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "measure-framerate-callbacks.hh"

#include "test-helpers.hh"

////////////////////////////////////////////////////////////////////////

std::vector<boost::function<bool()>> functions;
static unsigned int                  current     = 0;
static GtkWidget*                    drawingArea = nullptr;

////////////////////////////////////////////////////////////////////////
// Internals

static gboolean on_configure(GtkWidget* /*unused*/, GdkEventConfigure* /*unused*/, gpointer /*unused*/)
{
  // There should be a simpler way to do this...
  cairo_region_t*       r = gdk_window_get_visible_region(gtk_widget_get_window(drawingArea));
  cairo_rectangle_int_t rect;
  cairo_region_get_extents(r, &rect);

  drawingAreaWidth  = rect.width;
  drawingAreaHeight = rect.height;

  cairo_region_destroy(r);

  return FALSE;
}

static void on_hide(GtkWidget* /*unused*/, gpointer /*unused*/) { gtk_main_quit(); }

static gboolean on_expose(GtkWidget* widget, GdkEventExpose* /*unused*/, gpointer /*unused*/)
{
  cairo_region_t* re = cairo_region_create();

  GdkDrawingContext* dc;
  dc = gdk_window_begin_draw_frame(gtk_widget_get_window(widget), re);

  cairo_t* cr = gdk_drawing_context_get_cairo_context(dc);

  if(testData)
  {
    testData->redraw(cr);
  }

  gdk_window_end_draw_frame(gtk_widget_get_window(widget), dc);

  cairo_region_destroy(re);

  return FALSE;
}

static gboolean on_idle(gpointer /*unused*/)
{
  if(current >= functions.size())
  {
    return false;
  }

  if(!functions[current]())
  {
    current++;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////
// Externals

GtkWidget* create_window()
{
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Measure Framerate");
  gtk_window_maximize(GTK_WINDOW(window));

  g_signal_connect(static_cast<gpointer>(window), "hide", G_CALLBACK(on_hide), NULL);

  drawingArea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), drawingArea);
  g_signal_connect(static_cast<gpointer>(drawingArea), "draw", G_CALLBACK(on_expose), NULL);
  g_signal_connect(static_cast<gpointer>(drawingArea), "configure_event", G_CALLBACK(on_configure), NULL);

  gtk_widget_show(drawingArea);
  gtk_widget_show(window);

  return window;
}

void init() { gdk_threads_add_idle(on_idle, nullptr); }

void invalidate()
{
  GdkWindow* window = gtk_widget_get_window(drawingArea);
  gdk_window_invalidate_rect(window, nullptr, false);
}
