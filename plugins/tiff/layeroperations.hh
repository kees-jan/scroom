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

#ifndef _TIFF_LAYEROPERATIONS_HH
#define _TIFF_LAYEROPERATIONS_HH

#include <scroom/tiledbitmapinterface.hh>
#include <scroom/colormappable.hh>

// Avoid a circular reference...
class TiffPresentation;

class CommonOperations : public LayerOperations
{
protected:
  TiffPresentation* presentation;

public:
  CommonOperations(TiffPresentation* presentation);
  
  virtual ~CommonOperations()
  {}

  double mix(double d1, double d2, byte greyscale);
  void drawPixel(cairo_t* cr, int x, int y, int size, const Color& color);
  void drawPixel(cairo_t* cr, int x, int y, int size, const Color& c1, const Color& c2, byte greyscale);
  void fillRect(cairo_t* cr, int x, int y, int width, int height);
  void fillRect(cairo_t* cr, const GdkRectangle& area);
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual void initializeCairo(cairo_t* cr);
  virtual void drawState(cairo_t* cr, TileState s, GdkRectangle viewArea);
};

class Operations1bpp : public CommonOperations
{
public:
  Operations1bpp(TiffPresentation* presentation);
  virtual ~Operations1bpp()
  {}
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual int getBpp();
  virtual void draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom);
  virtual void reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y);
};

class Operations8bpp : public CommonOperations
{
public:
  Operations8bpp(TiffPresentation* presentation);
  virtual ~Operations8bpp()
  {}
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual int getBpp();
  virtual void draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom);
  virtual void reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y);
};

class Operations : public CommonOperations
{
private:
  const int bpp;
  const int pixelsPerByte;
  const int pixelOffset;
  const int pixelMask;

public:
  Operations(TiffPresentation* presentation, int bpp);
  
  virtual ~Operations()
  {}
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual int getBpp();
  virtual void draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom);
  virtual void reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y);
};


#endif