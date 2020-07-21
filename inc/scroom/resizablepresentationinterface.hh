/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <gdk/gdk.h>

#include <cairo.h>

#include <scroom/observable.hh>
#include <scroom/viewinterface.hh>

/**
 * Represent some 2D content that can assume any given size.
 */
class ResizablePresentationInterface
{
public:
  typedef boost::shared_ptr<ResizablePresentationInterface> Ptr;
  typedef boost::weak_ptr<ResizablePresentationInterface>   WeakPtr;

  virtual ~ResizablePresentationInterface() {}

  virtual void setRect(ViewInterface::WeakPtr const& vi, Scroom::Utils::Rectangle<double> const& rect) = 0;
};
