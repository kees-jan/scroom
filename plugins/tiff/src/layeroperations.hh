/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

  void setClip(cairo_t* cr, int x, int y, int width, int height);
  void setClip(cairo_t* cr, const GdkRectangle& area);
  void drawPixelValue(cairo_t* cr, int x, int y, int size, int value);
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual void initializeCairo(cairo_t* cr);
  virtual void drawState(cairo_t* cr, TileState s, GdkRectangle viewArea);

  virtual Scroom::Utils::Stuff cacheZoom(const Tile::Ptr tile, int zoom,
                                                Scroom::Utils::Stuff cache);
  virtual void draw(cairo_t* cr, const Tile::Ptr tile,
                    GdkRectangle tileArea, GdkRectangle viewArea, int zoom,
                    Scroom::Utils::Stuff cache);

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
  virtual Scroom::Utils::Stuff cache(const Tile::Ptr tile);
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
  virtual Scroom::Utils::Stuff cache(const Tile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y);

  virtual void draw(cairo_t* cr, const Tile::Ptr tile,
                    GdkRectangle tileArea, GdkRectangle viewArea, int zoom,
                    Scroom::Utils::Stuff cache);

};

class Operations : public CommonOperations
{
protected:
  const unsigned bpp;
  const unsigned pixelsPerByte;
  const unsigned pixelOffset;
  const unsigned pixelMask;

public:
  Operations(TiffPresentation* presentation, int bpp);
  
  virtual ~Operations()
  {}
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations
  
  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const Tile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y);
};

class OperationsColormapped : public Operations
{
public:
  OperationsColormapped(TiffPresentation* presentation, int bpp);

  virtual ~OperationsColormapped()
  {}

  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const Tile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y);
};


#endif
