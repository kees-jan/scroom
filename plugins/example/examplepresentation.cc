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

#include "examplepresentation.hh"

#include <unused.h>

ExamplePresentation::~ExamplePresentation()
{
}

GdkRectangle ExamplePresentation::getRect()
{
  GdkRectangle rect;
  rect.x=-500;
  rect.y=-500;
  rect.width=1000;
  rect.height=1000;

  return rect;
}

void ExamplePresentation::open(ViewInterface* viewInterface)
{
  UNUSED(viewInterface);
}

void ExamplePresentation::close(ViewInterface* vi)
{
  UNUSED(vi);
}

void ExamplePresentation::redraw(ViewInterface* vi, cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  UNUSED(vi);
  // char buffer[] = "Hello world!";
  // 
  // cairo_move_to(cr, 30, 30);
  // cairo_show_text(cr, buffer);

  double pp=1.0;
  if(zoom >=0)
    pp *= 1<<zoom;
  else
    pp /= 1<<(-zoom);

  // printf("One presentation pixel amounts to %f screen pixels\n", pp);

  int xorig = (int)(-presentationArea.x*pp);
  int yorig = (int)(-presentationArea.y*pp);

  for(int i=-500; i<500; i+=50)
  {
    cairo_move_to(cr, xorig-500*pp, yorig+i*pp);
    cairo_line_to(cr, xorig+500*pp, yorig+i*pp);
    cairo_move_to(cr, xorig+i*pp, yorig-500*pp);
    cairo_line_to(cr, xorig+i*pp, yorig+500*pp);
  }
  cairo_move_to(cr, xorig-500*pp, yorig-500*pp);
  cairo_line_to(cr, xorig+500*pp, yorig+500*pp);
  cairo_move_to(cr, xorig-500*pp, yorig+500*pp);
  cairo_line_to(cr, xorig+500*pp, yorig-500*pp);

  cairo_stroke(cr);
}

bool ExamplePresentation::getProperty(const std::string& name, std::string& value)
{
  UNUSED(name);
  UNUSED(value);
  
  return false;
}
bool ExamplePresentation::isPropertyDefined(const std::string& name)
{
  UNUSED(name);
    
  return false;
}

std::string ExamplePresentation::getTitle()
{
  return "";
}

