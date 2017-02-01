/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef _TIFFPRESENTATION_HH
#define _TIFFPRESENTATION_HH

#include <string>
#include <map>
#include <list>
#include <set>

#include <scroom/scroominterface.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/colormappable.hh>
#include <scroom/observable.hh>

typedef struct tiff TIFF;

class TiffPresentation : public SourcePresentation, public ColormapProvider,
                         public virtual Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<TiffPresentation> Ptr;

private:
  typedef std::set<ViewInterface::WeakPtr> Views;
  
  std::string fileName;
  TIFF* tif;
  int height;
  int width;
  TiledBitmapInterface::Ptr tbi;
  int bpp;
  std::map<std::string, std::string> properties;
  Views views;
  ColormapHelper::Ptr colormapHelper;
  
private:
  TiffPresentation();
  
public:
  virtual ~TiffPresentation();

  static Ptr create();

  /**
   * Called when this presentation should go away.
   *
   * Note that this doesn't happen automatically, since the
   * TiledBitmapInterface has a reference to this presentation, via
   * the LayerSpec, and possibly the SourcePresentation.
   */
  void destroy();

  bool load(const std::string& fileName);
  
  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  virtual GdkRectangle getRect();
  virtual void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, GdkRectangle presentationArea, int zoom);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();

  ////////////////////////////////////////////////////////////////////////
  // PresentationBase
  ////////////////////////////////////////////////////////////////////////

  virtual void viewAdded(ViewInterface::WeakPtr viewInterface);
  virtual void viewRemoved(ViewInterface::WeakPtr vi);
  virtual std::set<ViewInterface::WeakPtr> getViews();

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
  virtual void setColormap(Colormap::Ptr colormap);
  virtual Colormap::Ptr getOriginalColormap();
  virtual int getNumberOfColors();
  virtual Color getMonochromeColor();
  virtual void setMonochromeColor(const Color& c);
  virtual void setTransparentBackground();
  virtual void disableTransparentBackground();
  virtual bool getTransparentBackground();
  ////////////////////////////////////////////////////////////////////////
  // Helpers
  ////////////////////////////////////////////////////////////////////////
public:
  Colormap::Ptr getColormap();
  
};

class TiffPresentationWrapper : public PresentationBase, public Colormappable
{
public:
  typedef boost::shared_ptr<TiffPresentationWrapper> Ptr;
  
private:
  TiffPresentation::Ptr presentation;

private: 
  TiffPresentationWrapper();

public:
  static Ptr create();
  
  virtual ~TiffPresentationWrapper();

  bool load(const std::string& fileName);
  
  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  virtual GdkRectangle getRect();
  virtual void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, GdkRectangle presentationArea, int zoom);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();

  ////////////////////////////////////////////////////////////////////////
  // PresentationBase
  ////////////////////////////////////////////////////////////////////////

  virtual void viewAdded(ViewInterface::WeakPtr viewInterface);
  virtual void viewRemoved(ViewInterface::WeakPtr vi);
  virtual std::set<ViewInterface::WeakPtr> getViews();

  ////////////////////////////////////////////////////////////////////////
  // Colormappable
  ////////////////////////////////////////////////////////////////////////

  virtual void setColormap(Colormap::Ptr colormap);
  virtual Colormap::Ptr getOriginalColormap();
  virtual int getNumberOfColors();
  virtual Color getMonochromeColor();
  virtual void setMonochromeColor(const Color& c);
  virtual void setTransparentBackground();
  virtual void disableTransparentBackground();
  virtual bool getTransparentBackground();
};

#endif
