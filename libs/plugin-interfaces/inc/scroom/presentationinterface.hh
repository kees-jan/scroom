/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>
#include <set>

#include <gdk/gdk.h>

#include <cairo.h>

#include <scroom/context.hh>
#include <scroom/interface.hh>
#include <scroom/observable.hh>
#include <scroom/rectangle.hh>
#include <scroom/viewinterface.hh>

/**
 * Implement Viewable if you want to be able to receive events when a
 * View is created and/or deleted.
 *
 * You'll want to receive these events if you either want to influence
 * something on-screen (PresentationInterface, ColormapProvider), or
 * want to store data related to a View (TiledBitmapInterface).
 */
class Viewable : private Interface
{
public:
  using Ptr     = std::shared_ptr<Viewable>;
  using WeakPtr = std::weak_ptr<Viewable>;

public:
  /**
   * Gets called just after the View is created
   *
   * @pre Should be called from within a Gdk critical section
   *    (i.e. between gdk_threads_enter() and gdk_threads_leave()
   *    calls)
   */
  virtual void open(ViewInterface::WeakPtr vi) = 0;

  /**
   * Gets called just before the View is destroyed
   *
   * @pre Should be called from within a Gdk critical section
   *    (i.e. between gdk_threads_enter() and gdk_threads_leave()
   *    calls)
   */
  virtual void close(ViewInterface::WeakPtr vi) = 0;
};

using ViewObservable = Scroom::Utils::Observable<Viewable>;

/**
 * Represent some 2D content
 *
 * Anything that should be shown by Scroom in a window should
 * implement this interface.
 *
 * Objects implementing this interface are typically returned by
 * NewPresentationInterface::createNew() and OpenPresentationInterface::open().
 */
class PresentationInterface
  : public Viewable
  , public ViewObservable
{
public:
  using Ptr     = std::shared_ptr<PresentationInterface>;
  using WeakPtr = std::weak_ptr<PresentationInterface>;

  /** Return the dimensions of your presentation */
  virtual Scroom::Utils::Rectangle<double> getRect() = 0;

  /**
   * Draw the requested ara at the requested zoom level
   *
   * @param vi The ViewInterface on whose behalf the request is made
   * @param cr The context to draw the area on
   * @param presentationArea the area that is to be drawn. The given
   *    @c x and @c y coordinates should map on 0,0 of the given
   *    context @c cr.
   * @param zoom The requested zoom level. One pixel of your
   *    presentation should have size 2**@c zoom when drawn. @c zoom
   *    may be negative.
   */
  virtual void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom) = 0;

  /**
   * Return the value of the requested property
   *
   * @param[in] name The name of the requested property
   * @param[out] value The value of the requested property
   * @retval true if the property existed
   * @retval false if the property didn't exist
   */
  virtual bool getProperty(const std::string& name, std::string& value) = 0;

  /** Return true if the named property exists */
  virtual bool isPropertyDefined(const std::string& name) = 0;

  /** Return the title of the presentation */
  virtual std::string getTitle() = 0;

  virtual Scroom::Utils::Point<double>     getAspectRatio() const { return Scroom::Utils::make_point(1.0, 1.0); }
  virtual Scroom::Utils::Context::ConstPtr getContext() const = 0;
};

class PresentationBase : public PresentationInterface
{
public:
  // Viewable
  void open(ViewInterface::WeakPtr vi) override;
  void close(ViewInterface::WeakPtr vi) override;

protected:
  // ViewObservable
  void observerAdded(Viewable::Ptr const& viewable, Scroom::Bookkeeping::Token const& t) override;

protected:
  virtual void                                              viewAdded(ViewInterface::WeakPtr vi)   = 0;
  virtual void                                              viewRemoved(ViewInterface::WeakPtr vi) = 0;
  virtual Scroom::Utils::WeakKeySet<ViewInterface::WeakPtr> getViews()                             = 0;
};

class PresentationBaseSimple : public PresentationBase
{
private:
  Scroom::Utils::WeakKeySet<ViewInterface::WeakPtr> views;

private:
  void                                              viewAdded(ViewInterface::WeakPtr vi) final { views.insert(vi); }
  void                                              viewRemoved(ViewInterface::WeakPtr vi) final { views.erase(vi); }
  Scroom::Utils::WeakKeySet<ViewInterface::WeakPtr> getViews() final { return views; }
};

/**
 * Base class for something that composes several presentations.
 */
class Aggregate : private Interface
{
public:
  using Ptr = std::shared_ptr<Aggregate>;

public:
  virtual void addPresentation(PresentationInterface::Ptr const& presentation) = 0;
};
