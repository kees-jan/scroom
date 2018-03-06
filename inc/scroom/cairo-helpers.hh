/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/color.hh>

#include <list>

#include <gdk/gdk.h>
#include <cairo.h>

#include <scroom/rectangle.hh>

namespace Colors
{
  extern const Color OUT_OF_BOUNDS;
}

void setClip(cairo_t* cr, int x, int y, int width, int height);
void setClip(cairo_t* cr, const GdkRectangle& area);
void drawRectangle(cairo_t* cr, Color const& c, Rectangle<double> const& viewArea);
void drawOutOfBoundsWithBackground(cairo_t* cr, 
                                   Rectangle<int> const& requestedPresentationArea,
                                   Rectangle<int> const& actualPresentationArea, double pixelSize);
void drawOutOfBoundsWithBackgroundColor(cairo_t* cr, const Color& background,
                                   Rectangle<int> const& requestedPresentationArea,
                                   Rectangle<int> const& actualPresentationArea, double pixelSize);
void drawOutOfBoundsWithoutBackground(cairo_t* cr,
                                      Rectangle<int> const& requestedPresentationArea,
                                      Rectangle<int> const& actualPresentationArea, double pixelSize);
double pixelSizeFromZoom(int zoom);
