/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "examplepresentation.hh"

#include <cmath>

#include <gdk/gdk.h>

#include <scroom/cairo-helpers.hh>


ExamplePresentation::ExamplePresentation()
  : context(Scroom::Utils::Context::Create())
{
  fillPattern();
}

ExamplePresentation::~ExamplePresentation() { cairo_pattern_destroy(pattern); }

void ExamplePresentation::fillPattern()
{
  cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_A1, 1010, 1010);
  cairo_t*         cr      = cairo_create(surface);

  const int xorig = 505;
  const int yorig = 505;

  for(int i = -500; i <= 500; i += 50)
  {
    cairo_move_to(cr, xorig - 500, yorig + i);
    cairo_line_to(cr, xorig + 500, yorig + i);
    cairo_move_to(cr, xorig + i, yorig - 500);
    cairo_line_to(cr, xorig + i, yorig + 500);
  }
  cairo_move_to(cr, xorig - 500, yorig - 500);
  cairo_line_to(cr, xorig + 500, yorig + 500);
  cairo_move_to(cr, xorig - 500, yorig + 500);
  cairo_line_to(cr, xorig + 500, yorig - 500);

  cairo_stroke(cr);

  pattern = cairo_pattern_create_for_surface(cairo_get_target(cr));

  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}

Scroom::Utils::Rectangle<double> ExamplePresentation::getRect() { return {-500, -500, 1000, 1000}; }

void ExamplePresentation::open(ViewInterface::WeakPtr /*viewInterface*/) {}

void ExamplePresentation::close(ViewInterface::WeakPtr /*vi*/) {}

void ExamplePresentation::redraw(ViewInterface::Ptr const& /*vi*/, cairo_t* cr, Scroom::Utils::Rectangle<double> pa, int zoom)
{
  cairo_rectangle_int_t const presentationArea = pa.toGdkRectangle();
  const double                pp               = pixelSizeFromZoom(zoom);
  const double                scale            = pow(2, -zoom);

  Scroom::Utils::Rectangle<double> const actualPresentationArea = getRect();
  drawOutOfBoundsWithBackground(cr, pa, actualPresentationArea, pp);

  const int xorig = static_cast<int>(-presentationArea.x * pp);
  const int yorig = static_cast<int>(-presentationArea.y * pp);

  cairo_matrix_t m;
  cairo_matrix_init_translate(&m, 505, 505);
  cairo_matrix_scale(&m, scale, scale);
  cairo_matrix_translate(&m, -xorig, -yorig);
  cairo_pattern_set_matrix(pattern, &m);
  cairo_mask(cr, pattern);

  // cairo_set_source_surface(cr, surface, xorig-505, yorig-505);
  // cairo_paint(cr);
}

bool ExamplePresentation::getProperty(const std::string& /*name*/, std::string& /*value*/) { return false; }

bool ExamplePresentation::isPropertyDefined(const std::string& /*name*/) { return false; }

std::string                      ExamplePresentation::getTitle() { return ""; }
Scroom::Utils::Context::ConstPtr ExamplePresentation::getContext() const { return context; }
