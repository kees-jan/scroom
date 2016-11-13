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

#ifndef _RESIZABLEPRESENTATIONINTERFACE_H
#define _RESIZABLEPRESENTATIONINTERFACE_H

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <gdk/gdk.h>
#include <cairo.h>

#include <set>

#include <scroom/viewinterface.hh>
#include <scroom/observable.hh>

/**
 * Represent some 2D content that can assume any given size.
 */
class ResizablePresentationInterface
{
public:
  typedef boost::shared_ptr<ResizablePresentationInterface> Ptr;
  typedef boost::weak_ptr<ResizablePresentationInterface> WeakPtr;
 
  virtual ~ResizablePresentationInterface() {}

  virtual void setRect(ViewInterface::WeakPtr const& vi, GdkRectangle const& rect)=0;
};

#endif
