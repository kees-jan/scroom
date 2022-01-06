/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <gdk/gdk.h>

#include <cairo.h>

#include <scroom/interface.hh>
#include <scroom/observable.hh>
#include <scroom/viewinterface.hh>

/**
 * Represent some 2D content that can assume any given size.
 */
class ResizablePresentationInterface : private Interface
{
public:
  using Ptr     = boost::shared_ptr<ResizablePresentationInterface>;
  using WeakPtr = boost::weak_ptr<ResizablePresentationInterface>;

  virtual void setRect(ViewInterface::WeakPtr const& vi, Scroom::Utils::Rectangle<double> const& rect) = 0;
};
