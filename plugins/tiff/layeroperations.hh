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

#include <tiledbitmapinterface.hh>
#include <colormappable.hh>

// Avoid a circular reference...
class TiffPresentation;

class CommonOperations : public LayerOperations
{
public:
  virtual ~CommonOperations()
  {}

  void drawPixel(cairo_t* cr, int x, int y, int size, byte greyShade=255);
  void drawPixel(cairo_t* cr, int x, int y, int size, double greyShade);
  void drawPixel(cairo_t* cr, int x, int y, int size, const Color& color);
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

  TiffPresentation* presentation;
  
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
