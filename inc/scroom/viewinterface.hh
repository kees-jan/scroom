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
#include <scroom/rectangle.hh>

class PresentationInterface;
class ViewInterface;

/**
 * Structure that represents a selection made
 * by the user.
 */
struct Selection
{
public:
  typedef boost::shared_ptr<Selection> Ptr;

public:
  GdkPoint start;
  GdkPoint end;

public:
  Selection(int x, int y) { start.x=x; start.y=y; end=start; }
  Selection(GdkPoint point) : start(point), end(point) {}

  bool endsAt(GdkPoint p) { return end.x==p.x && end.y==p.y; }

  int width() { return abs(end.x-start.x); }
  int height() { return abs(end.y-start.y); }
  double length() { return std::hypot(width(), height()); }
};

/**
 * Interface provided to something that wants to
 * draw on top of the current presentation.
 */
class PostRenderer
{
public:
  typedef boost::shared_ptr<PostRenderer> Ptr;

public:
  virtual ~PostRenderer() {}

  /**
   * This function is called after the presentation
   * finishes redrawing.
   *
   * @param vi The ViewInterface on whose behalf the request is made
   * @param cr The context to with, the origin of the context is
   *    translated to be the same as the origin of the presentation.
   * @param presentationArea the area that is to be drawn. The given
   *    @c x and @c y coordinates should map on 0,0 of the given
   *    context @c cr.
   * @param zoom The requested zoom level. One pixel should have
   *    size 2**@c zoom when drawn. @c zoom may be negative.
   */
  virtual void render(boost::shared_ptr<ViewInterface> const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom)=0;
};

/**
 * Interface provided to something that wants to be
 * notified of selection updates.
 * 
 * Whenever the user selection changes, the appropriate
 * functions are called with either the starting point
 * or the full selection as argument.
 *
 * @see Selection
 */
class SelectionListener
{
public:
  typedef boost::shared_ptr<SelectionListener> Ptr;

public:
  virtual ~SelectionListener() {}

  /**
   * This function is called whenever the user clicks
   * a view. The point that is clicked is passed as
   * an argument.
   * 
   * The passed point is a point in the presentation
   * coordinate space.
   */
  virtual void onSelectionStart(GdkPoint start, boost::shared_ptr<ViewInterface> view)=0;

  /**
   * This function is called whenever the selection
   * updates. That is, whenever the user moves the
   * mouse while keeping the mouse button pressed.
   * 
   * The updated selection is passed as an argument.
   * 
   * @see Selection
   */
  virtual void onSelectionUpdate(Selection::Ptr selection, boost::shared_ptr<ViewInterface> view)=0;

  /**
   * This function is called whenever the selection
   * ends. That is, whenever the user releases the
   * mouse button that was pressed.
   * 
   * The final selection is passed as an argument.
   * 
   * @see Selection
   */
  virtual void onSelectionEnd(Selection::Ptr selection, boost::shared_ptr<ViewInterface> view)=0;
};

/**
 * Interface provided to something that wants to
 * listener to enable/disable events from its
 * tool button.
 */
class ToolStateListener
{
public:
  typedef boost::shared_ptr<ToolStateListener> Ptr;

public:
  virtual ~ToolStateListener() {}

  /**
   * This function is called whenever the tool
   * button is selected.
   */
  virtual void onEnable()=0;

  /**
   * Then function is called whenever the tool
   * button is deselected.
   */
  virtual void onDisable()=0;
};

/**
 * Interface provided to something Viewable
 *
 * Internally, scroom uses a View to represent the fact that something
 * (typically a presentation) is visible on the screen. Being a
 * presentation, you typically want to influence whatever is being
 * shown. This interface allows you to do so.
 *
 * @see PresentationInterface, Viewable, SelectionListener, PostRenderer, ToolStateListener
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
   * Register a SelectionListener to be updated whenever the
   * user selects a region . When the user changes the selection,
   * various functions on the given instance are called.
   * 
   * @see SelectionListener
   */
  virtual void registerSelectionListener(SelectionListener::Ptr)=0;

  /**
   * Register a postrenderer to be updated whenever a redraw
   * occurs. When this happens, the 'render' function
   * gets called on the instance that is passed to
   * the given instance.
   * 
   * Note that the order in which different registered instances
   * get updated is the order in which they register to the
   * view. This order remains constant throughout the view's
   * lifetime.
   * 
   * @see PostRenderer
   */
  virtual void registerPostRenderer(PostRenderer::Ptr)=0;

  /**
   * Sets the status message in the status bar of the application.
   */
  virtual void setStatusMessage(const std::string&)=0;

  /**
   * Returns a shared pointer to the current presentation.
   * 
   * @see PresentationInterface
   */
  virtual boost::shared_ptr<PresentationInterface> getCurrentPresentation()=0;

  /**
   * Adds a new tool button to the toolbar. The given ToolStateListener
   * will be informed when the tool is turned on or off. Only one
   * tool will be active at the same time.
   *
   * @see ToolStateListener
   */
  virtual void addToolButton(GtkToggleButton*, ToolStateListener::Ptr)=0;
};

