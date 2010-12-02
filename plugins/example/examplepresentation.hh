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

#ifndef _EXAMPLEPRESENTATION_HH
#define _EXAMPLEPRESENTATION_HH

#include <scroom/presentationinterface.hh>

class ExamplePresentation : public PresentationInterface
{
public:
  virtual ~ExamplePresentation();

  virtual GdkRectangle getRect();
  virtual void open(ViewInterface* viewInterface);
  virtual void redraw(ViewInterface* vi, cairo_t* cr, GdkRectangle presentationArea, int zoom);
  virtual void close(ViewInterface* vi);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();
};

#endif
