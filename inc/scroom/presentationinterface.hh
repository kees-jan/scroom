/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

#ifndef _PRESENTATIONINTERFACE_H
#define _PRESENTATIONINTERFACE_H

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <gdk/gdk.h>
#include <cairo.h>

#include <scroom/viewinterface.hh>

class Viewable
{
public:
  virtual ~Viewable() {}
    
  virtual void open(ViewInterface* vi)=0;
  virtual void close(ViewInterface* vi)=0;
};

class PresentationInterface : public Viewable
{
public:
  typedef boost::shared_ptr<PresentationInterface> Ptr;
  typedef boost::weak_ptr<PresentationInterface> WeakPtr;
 
  virtual ~PresentationInterface() {}

  virtual GdkRectangle getRect()=0;
  virtual void redraw(ViewInterface* vi, cairo_t* cr, GdkRectangle presentationArea, int zoom)=0;
  virtual bool getProperty(const std::string& name, std::string& value)=0;
  virtual bool isPropertyDefined(const std::string& name)=0;
  virtual std::string getTitle()=0;
  
};

#endif
