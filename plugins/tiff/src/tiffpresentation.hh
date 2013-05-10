/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2013 Kees-Jan Dijkzeul
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

#ifndef _TIFFPRESENTATION_HH
#define _TIFFPRESENTATION_HH

#include <string>
#include <map>
#include <list>

#include <scroom/scroominterface.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/colormappable.hh>
#include <scroom/observable.hh>

typedef struct tiff TIFF;

class TiffPresentation : public PresentationInterface, public SourcePresentation, public Colormappable, public ColormapProvider
{
public:
  typedef boost::shared_ptr<TiffPresentation> Ptr;

private:
  typedef bool Dummy;
  typedef std::map<ViewInterface::WeakPtr, Dummy> Views;
  static const Dummy dummy;
  
  std::string fileName;
  TIFF* tif;
  int height;
  int width;
  TiledBitmapInterface::Ptr tbi;
  LayerSpec ls;
  int bpp;
  std::map<std::string, std::string> properties;
  Views views;
  Colormap::Ptr originalColormap;
  Colormap::Ptr colormap;
  
public:

  TiffPresentation();
  virtual ~TiffPresentation();

  bool load(std::string fileName);
  
  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  virtual GdkRectangle getRect();
  virtual void open(ViewInterface::WeakPtr viewInterface);
  virtual void redraw(ViewInterface::Ptr vi, cairo_t* cr, GdkRectangle presentationArea, int zoom);
  virtual void close(ViewInterface::WeakPtr vi);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();

  ////////////////////////////////////////////////////////////////////////
  // SourcePresentation
  ////////////////////////////////////////////////////////////////////////
public:
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);
  virtual void done();
 
  ////////////////////////////////////////////////////////////////////////
  // Colormappable
  ////////////////////////////////////////////////////////////////////////

public:
  virtual void observerAdded(Viewable::Ptr observer, Scroom::Bookkeeping::Token token);
  void setColormap(Colormap::Ptr colormap);
  Colormap::Ptr getOriginalColormap();
  int getNumberOfColors();

  ////////////////////////////////////////////////////////////////////////
  // Helpers
  ////////////////////////////////////////////////////////////////////////
public:
  Colormap::Ptr getColormap();
  
};

#endif
