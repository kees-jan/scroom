/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "measure-framerate-callbacks.hh"

#include "test-helpers.hh"

#include <scroom/unused.hh>

////////////////////////////////////////////////////////////////////////

std::vector<boost::function<bool ()> > functions;
static unsigned int current=0;
static GtkWidget* drawingArea=NULL;

////////////////////////////////////////////////////////////////////////
// Internals

static gboolean on_configure(GtkWidget*, GdkEventConfigure*, gpointer)
{
  // There should be a simpler way to do this...
  GdkRegion* r = gdk_drawable_get_visible_region(GDK_DRAWABLE(gtk_widget_get_window(drawingArea)));
  GdkRectangle rect;
  gdk_region_get_clipbox(r, &rect);

  drawingAreaWidth = rect.width;
  drawingAreaHeight = rect.height;

  gdk_region_destroy(r);

  return FALSE;
}

static void on_hide(GtkWidget*, gpointer)
{
  gtk_main_quit();
}

static gboolean on_expose(GtkWidget* widget, GdkEventExpose*, gpointer)
{
  cairo_t* cr = gdk_cairo_create(widget->window);

  if(testData)
    testData->redraw(cr);

  cairo_destroy(cr);
  return FALSE;
}

static gboolean on_idle(gpointer)
{
  if(current>=functions.size())
    return false;

  if(!functions[current]())
    current++;

  return true;
}

////////////////////////////////////////////////////////////////////////
// Externals

GtkWidget* create_window()
{
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Measure Framerate");
  gtk_window_maximize(GTK_WINDOW(window));

  g_signal_connect (static_cast<gpointer>(window), "hide", G_CALLBACK (on_hide), NULL);

  drawingArea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), drawingArea);
  g_signal_connect (static_cast<gpointer>(drawingArea), "expose_event", G_CALLBACK (on_expose), NULL);
  g_signal_connect (static_cast<gpointer>(drawingArea), "configure_event", G_CALLBACK (on_configure), NULL);

  gtk_widget_show(drawingArea);
  gtk_widget_show(window);

  return window;
}

void init()
{
  gtk_idle_add(on_idle, NULL);
}

void invalidate()
{
  GdkWindow* window = gtk_widget_get_window(drawingArea);
  gdk_window_invalidate_rect(window, NULL, false);

  gdk_threads_enter();
  gdk_window_process_all_updates();
  gdk_threads_leave();
}
