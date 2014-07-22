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

#include "transparentoverlaypresentation.hh"

#include <math.h>

#include <scroom/unused.hh>

TransparentOverlayPresentation::TransparentOverlayPresentation()
  : pattern(NULL)
{
  fillPattern();
}

TransparentOverlayPresentation::~TransparentOverlayPresentation()
{
  cairo_pattern_destroy(pattern);
}

void TransparentOverlayPresentation::fillPattern()
{
  cairo_surface_t*  surface = cairo_image_surface_create(CAIRO_FORMAT_A1, 1010, 1010);
  cairo_t* cr = cairo_create(surface);

  int xorig = 505;
  int yorig = 505;

  for(int i=-500; i<500; i+=50)
  {
    cairo_move_to(cr, xorig-500, yorig+i);
    cairo_line_to(cr, xorig+500, yorig+i);
    cairo_move_to(cr, xorig+i, yorig-500);
    cairo_line_to(cr, xorig+i, yorig+500);
  }
  cairo_move_to(cr, xorig-500, yorig-500);
  cairo_line_to(cr, xorig+500, yorig+500);
  cairo_move_to(cr, xorig-500, yorig+500);
  cairo_line_to(cr, xorig+500, yorig-500);

  cairo_stroke(cr);

  pattern = cairo_pattern_create_for_surface(cairo_get_target(cr));
  
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}

GdkRectangle TransparentOverlayPresentation::getRect()
{
  GdkRectangle rect;
  rect.x=-500;
  rect.y=-500;
  rect.width=1000;
  rect.height=1000;

  return rect;
}

void TransparentOverlayPresentation::open(ViewInterface::WeakPtr viewInterface)
{
  UNUSED(viewInterface);
}

void TransparentOverlayPresentation::close(ViewInterface::WeakPtr vi)
{
  UNUSED(vi);
}

void TransparentOverlayPresentation::redraw(ViewInterface::Ptr vi, cairo_t* cr, GdkRectangle presentationArea, int zoom)
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

  double scale = pow(2, -zoom);

  int xorig = (int)(-presentationArea.x*pp);
  int yorig = (int)(-presentationArea.y*pp);

  cairo_matrix_t m;
  cairo_matrix_init_translate(&m, 505, 505);
  cairo_matrix_scale(&m, scale, scale);
  cairo_matrix_translate(&m, -xorig, -yorig);
  cairo_pattern_set_matrix(pattern, &m);
  cairo_mask(cr, pattern);

  // cairo_set_source_surface(cr, surface, xorig-505, yorig-505);
  // cairo_paint(cr);
}

bool TransparentOverlayPresentation::getProperty(const std::string& name, std::string& value)
{
  UNUSED(name);
  UNUSED(value);
  
  return false;
}
bool TransparentOverlayPresentation::isPropertyDefined(const std::string& name)
{
  UNUSED(name);
    
  return false;
}

std::string TransparentOverlayPresentation::getTitle()
{
  return "";
}

