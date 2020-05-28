/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <stdlib.h>

#include <string>
#include <cmath>

#include <gtk/gtk.h>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/progressinterface.hh>
#include <scroom/stuff.hh>

struct Selection
{
public:
  GdkPoint start;
  GdkPoint end;

public:
  Selection(int x, int y) { start.x=x; start.y=y; end=start; }
  Selection(GdkPoint point) : start(point), end(point) {}

  bool endsAt(GdkPoint p) { return end.x==p.x && end.y==p.y; }

  int width() { return abs(end.x-start.x); }
  int height() { return abs(end.y-start.y); }
  double length() { return std::sqrt(std::pow(double(width()),2) + std::pow(double(height()),2)); }
};

class PostRenderer
{
public:
  typedef boost::shared_ptr<PostRenderer> Ptr;

public:
  virtual ~PostRenderer() {}

  virtual void render(cairo_t* cr)=0;
};

class SelectionListener
{
public:
  typedef boost::shared_ptr<SelectionListener> Ptr;

public:
  virtual ~SelectionListener() {}

  virtual void onSelectionStart(GdkPoint start)=0;
  virtual void onSelectionUpdate(Selection* selection)=0;
  virtual void onSelectionEnd(Selection* selection)=0;
};

// There is no documentation on values 4 and 5, so
// they are not included here.
enum class MouseButton : uint {
  PRIMARY = 1,
  MIDDLE = 2,
  SECONDARY = 3
};

/**
 * Interface provided to something Viewable
 *
 * Internally, scroom uses a View to represent the fact that something
 * (typically a presentation) is visible on the screen. Being a
 * presentation, you typically want to influence whatever is being
 * shown. This interface allows you to do so.
 *
 * @see PresentationInterface, Viewable
 */
class ViewInterface
{
public:
  typedef boost::shared_ptr<ViewInterface> Ptr;
  typedef boost::weak_ptr<ViewInterface> WeakPtr;

public:
  virtual ~ViewInterface() {}

  /**
   * Request that the window content is redrawn.
   *
   * @pre Should be called from within a Gdk critical section
   *    (i.e. between gdk_threads_enter() and gdk_threads_leave()
   *    calls)
   */
  virtual void invalidate()=0;

  /**
   * Return a pointer to the progess interface associated with the View
   *
   * @note The progress bar should only be manipulated from within a
   *    Gdk critical section (i.e. between gdk_threads_enter() and
   *    gdk_threads_leave() calls)
   */
  virtual ProgressInterface::Ptr getProgressInterface()=0;

  /**
   * Request that the given widget be added to the sidebar.
   *
   * @pre Should be called from within a Gdk critical section
   *    (i.e. between gdk_threads_enter() and gdk_threads_leave()
   *    calls)
   */
  virtual void addSideWidget(std::string title, GtkWidget* w)=0;

  /**
   * Request that the given widget be removed from the sidebar.
   *
   * @pre Should be called from within a Gdk critical section
   *    (i.e. between gdk_threads_enter() and gdk_threads_leave()
   *    calls)
   */
  virtual void removeSideWidget(GtkWidget* w)=0;

  /**
   * Request that the given tool item be added to the toolbar.
   *
   * @pre Should be called from within a Gdk critical section
   *    (i.e. between gdk_threads_enter() and gdk_threads_leave()
   *    calls)
   */
  virtual void addToToolbar(GtkToolItem* ti)=0;

  /**
   * Request that the given tool item be removed from the toolbar.
   *
   * @pre Should be called from within a Gdk critical section
   *    (i.e. between gdk_threads_enter() and gdk_threads_leave()
   *    calls)
   */
  virtual void removeFromToolbar(GtkToolItem* ti)=0;

  /**
   * Enable panning the view.
   */
  virtual void setPanning()
  {
  }

  /**
   * Disable panning the view.
   */
  virtual void unsetPanning()
  {
  }

  /**
   * Register a SelectionListener to be updated whenever the
   * user selects a region using the given mouse button. When
   * the user changes the selection, the function
   * 'onSelection(Selection* selection)' is called on the
   * given object.
   * 
   * @see SelectionListener
   */
  virtual void registerSelectionListener(SelectionListener::Ptr, MouseButton)
  {
  }

  /**
   * Register a postrenderer.
   */
  virtual void registerPostRenderer(PostRenderer::Ptr)
  {
  }

  /**
   * Sets the status message in the status bar of the application.
   */
  virtual void setStatusMessage(const std::string&)
  {
  }

  /**
   * Converts a point on screen to a presentation pixel.
   */
  virtual GdkPoint presentationPointToWindowPoint(GdkPoint)
  {
    return {0, 0};
  }

  /**
   * Returns a shared pointer to the current presentation.
   *
   * The actual return type should be PresentationInterface::Ptr,
   * but including the required header file causes a cyclic include.
   * This workaround means that you have to explicitly cast
   * the result of this function to PresentationInterface::Ptr.
   */
  virtual Scroom::Utils::Stuff getCurrentPresentation()
  {
    return nullptr;
  }
};

