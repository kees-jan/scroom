/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/cairo-helpers.hh>

#include <iostream>

#include <boost/assign/list_of.hpp>

namespace Colors
{
  const Color OUT_OF_BOUNDS(0.75, 0.75, 1); // Blue
  const Color IN_BOUNDS(1,1,1); // White
}


void setClip(cairo_t* cr, int x, int y, int width, int height)
{
  cairo_move_to(cr, x, y);
  cairo_line_to(cr, x+width, y);
  cairo_line_to(cr, x+width, y+height);
  cairo_line_to(cr, x, y+height);
  cairo_line_to(cr, x, y);
  cairo_clip(cr);
}

void setClip(cairo_t* cr, const GdkRectangle& area)
{
  setClip(cr, area.x, area.y, area.width, area.height);
}

void drawRectangle(cairo_t* cr, Color const& c, Rectangle<double> const& viewArea)
{
  cairo_save(cr);

  c.setColor(cr);
  setClip(cr, viewArea);

  cairo_paint(cr);

  cairo_restore(cr);
}

void drawOutOfBoundsWithBackground(cairo_t* cr, 
                                   Rectangle<int> const& requestedPresentationArea,
                                   Rectangle<int> const& actualPresentationArea, double pixelSize)
{
  drawRectangle(cr, Colors::OUT_OF_BOUNDS, pixelSize*(requestedPresentationArea.moveTo(0,0)));
  drawRectangle(cr, Colors::IN_BOUNDS,
                pixelSize*(actualPresentationArea - requestedPresentationArea.getTopLeft()));
}

void drawOutOfBoundsWithoutBackground(cairo_t* cr,
                                      Rectangle<int> const& requestedPresentationArea,
                                      Rectangle<int> const& actualPresentationArea, double pixelSize)
{
  std::list<Rectangle<int> > border = boost::assign::list_of
    (requestedPresentationArea.leftOf(actualPresentationArea.getLeftPos()))
    (requestedPresentationArea.rightOf(actualPresentationArea.getRightPos()))
    (requestedPresentationArea.above(actualPresentationArea.getTopPos()))
    (requestedPresentationArea.below(actualPresentationArea.getBottomPos()));

  for(const Rectangle<int>& r: border)
  {
    if(!r.isEmpty())
    {
      drawRectangle(cr, Colors::OUT_OF_BOUNDS, pixelSize*(r - requestedPresentationArea.getTopLeft()));
    }
  }
}

double pixelSizeFromZoom(int zoom)
{
  double pixelSize=1.0;
  if(zoom >=0)
    pixelSize *= 1<<zoom;
  else
    pixelSize /= 1<<(-zoom);

  return pixelSize;
}
