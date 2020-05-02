/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "examplepresentation.hh"

#include <math.h>

#include <scroom/cairo-helpers.hh>
#include <scroom/unused.hh>

ExamplePresentation::ExamplePresentation()
  : pattern(NULL)
{
  fillPattern();
}

ExamplePresentation::~ExamplePresentation()
{
  cairo_pattern_destroy(pattern);
}

void ExamplePresentation::fillPattern()
{
  cairo_surface_t*  surface = cairo_image_surface_create(CAIRO_FORMAT_A1, 1010, 1010);
  cairo_t* cr = cairo_create(surface);

  int xorig = 505;
  int yorig = 505;

  for(int i=-500; i<=500; i+=50)
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

Scroom::Utils::Rectangle<double> ExamplePresentation::getRect()
{
  GdkRectangle rect;
  rect.x=-500;
  rect.y=-500;
  rect.width=1000;
  rect.height=1000;

  return rect;
}

void ExamplePresentation::open(ViewInterface::WeakPtr viewInterface)
{
  UNUSED(viewInterface);
}

void ExamplePresentation::close(ViewInterface::WeakPtr vi)
{
  UNUSED(vi);
}

void ExamplePresentation::redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> pa, int zoom)
{
  GdkRectangle presentationArea = pa.toGdkRectangle();
  UNUSED(vi);
  double pp=pixelSizeFromZoom(zoom);
  double scale = pow(2, -zoom);

  Scroom::Utils::Rectangle<double> actualPresentationArea = getRect();
  drawOutOfBoundsWithBackground(cr, presentationArea, actualPresentationArea, pp);

  int xorig = static_cast<int>(-presentationArea.x*pp);
  int yorig = static_cast<int>(-presentationArea.y*pp);

  cairo_matrix_t m;
  cairo_matrix_init_translate(&m, 505, 505);
  cairo_matrix_scale(&m, scale, scale);
  cairo_matrix_translate(&m, -xorig, -yorig);
  cairo_pattern_set_matrix(pattern, &m);
  cairo_mask(cr, pattern);

  // cairo_set_source_surface(cr, surface, xorig-505, yorig-505);
  // cairo_paint(cr);
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

