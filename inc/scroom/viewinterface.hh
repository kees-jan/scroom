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

#ifndef _VIEWINTERFACE_HH
#define _VIEWINTERFACE_HH

#include <string>

#include <gtk/gtk.h>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

/**
 * Interface used for reporting progress information
 */
class ProgressInterface
{
public:
  typedef enum
    {
      IDLE,
      WAITING,
      WORKING,
      FINISHED
    } State;

  virtual ~ProgressInterface() {}
  
  virtual void setState(State s)=0;
  virtual void setProgress(double d)=0;
  virtual void setProgress(int done, int total)=0;
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
  virtual ProgressInterface* getProgressInterface()=0;

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
};


#endif
