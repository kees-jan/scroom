/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>

#include <gdk/gdk.h>

#include <cairo.h>

#include <scroom/color.hh>
#include <scroom/rectangle.hh>

namespace Colors
{
  extern const Color OUT_OF_BOUNDS;
}

void   traceRectangleContour(cairo_t* cr, Scroom::Utils::Rectangle<double> const& viewArea);
void   drawRectangleContour(cairo_t* cr, Scroom::Utils::Rectangle<double> const& viewArea);
void   drawRectangleContour(cairo_t* cr, Color const& c, Scroom::Utils::Rectangle<double> const& viewArea);
void   setClip(cairo_t* cr, double x, double y, double width, double height);
void   setClip(cairo_t* cr, const Scroom::Utils::Rectangle<double>& area);
void   drawRectangle(cairo_t* cr, Color const& c, Scroom::Utils::Rectangle<double> const& viewArea);
void   drawOutOfBoundsWithBackground(cairo_t*                                cr,
                                     Scroom::Utils::Rectangle<double> const& requestedPresentationArea,
                                     Scroom::Utils::Rectangle<double> const& actualPresentationArea,
                                     double                                  pixelSize);
void   drawOutOfBoundsWithBackgroundColor(cairo_t*                                cr,
                                          const Color&                            background,
                                          Scroom::Utils::Rectangle<double> const& requestedPresentationArea,
                                          Scroom::Utils::Rectangle<double> const& actualPresentationArea,
                                          double                                  pixelSize);
void   drawOutOfBoundsWithoutBackground(cairo_t*                                cr,
                                        Scroom::Utils::Rectangle<double> const& requestedPresentationArea,
                                        Scroom::Utils::Rectangle<double> const& actualPresentationArea,
                                        double                                  pixelSize);
double pixelSizeFromZoom(int zoom);
