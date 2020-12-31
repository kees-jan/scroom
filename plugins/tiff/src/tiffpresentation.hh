/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>
#include <set>
#include <string>

#include <scroom/colormappable.hh>
#include <scroom/observable.hh>
#include <scroom/pipetteviewinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/scroominterface.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/transformpresentation.hh>

typedef struct tiff TIFF;

class TiffPresentation
  : public SourcePresentation
  , public virtual Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<TiffPresentation> Ptr;

private:
  typedef std::set<ViewInterface::WeakPtr> Views;

  std::string                        fileName;
  TIFF*                              tif;
  int                                height;
  int                                width;
  TiledBitmapInterface::Ptr          tbi;
  int                                bps;
  int                                spp;
  std::map<std::string, std::string> properties;
  Views                              views;
  ColormapHelper::Ptr                colormapHelper;
  TransformationData::Ptr            transformationData;

  PipetteLayerOperations::Ptr pipetteLayerOperation;

private:
  TiffPresentation();

public:
  virtual ~TiffPresentation();
  TiffPresentation(const TiffPresentation&) = delete;
  TiffPresentation(TiffPresentation&&)      = delete;
  TiffPresentation operator=(const TiffPresentation&) = delete;
  TiffPresentation operator=(TiffPresentation&&) = delete;

  static Ptr create();

  /**
   * Called when this presentation should go away.
   *
   * Note that this doesn't happen automatically, since the
   * TiledBitmapInterface has a reference to this presentation, via
   * the SourcePresentation.
   */
  void destroy();

  bool                    load(const std::string& fileName);
  TransformationData::Ptr getTransformationData() const;

  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  virtual Scroom::Utils::Rectangle<double> getRect();
  virtual void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();

  ////////////////////////////////////////////////////////////////////////
  // PresentationBase
  ////////////////////////////////////////////////////////////////////////

  virtual void                             viewAdded(ViewInterface::WeakPtr viewInterface);
  virtual void                             viewRemoved(ViewInterface::WeakPtr vi);
  virtual std::set<ViewInterface::WeakPtr> getViews();

  ////////////////////////////////////////////////////////////////////////
  // SourcePresentation
  ////////////////////////////////////////////////////////////////////////
public:
  void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) override;
  void done() override;

  ////////////////////////////////////////////////////////////////////////
  // PipetteViewInterface
  ////////////////////////////////////////////////////////////////////////
public:
  /**
   * Returns the average pixel values for each component, contained in the area
   *
   * @param area selected area to get the pixel values from
   */
  virtual PipetteLayerOperations::PipetteColor getPixelAverages(Scroom::Utils::Rectangle<int> area);

  ////////////////////////////////////////////////////////////////////////
  // Colormappable
  ////////////////////////////////////////////////////////////////////////

public:
  virtual void          setColormap(Colormap::Ptr colormap);
  virtual Colormap::Ptr getOriginalColormap();
  virtual int           getNumberOfColors();
  virtual Color         getMonochromeColor();
  virtual void          setMonochromeColor(const Color& c);
  virtual void          setTransparentBackground();
  virtual void          disableTransparentBackground();
  virtual bool          getTransparentBackground();

private:
  void clearCaches();
};

class TiffPresentationWrapper
  : public PresentationBase
  , public Colormappable
  , public PipetteViewInterface
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
  TiffPresentationWrapper(const TiffPresentationWrapper&) = delete;
  TiffPresentationWrapper(TiffPresentationWrapper&&)      = delete;
  TiffPresentationWrapper operator=(const TiffPresentationWrapper&) = delete;
  TiffPresentationWrapper operator=(TiffPresentationWrapper&&) = delete;

  bool                    load(const std::string& fileName);
  TransformationData::Ptr getTransformationData() const;

  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  Scroom::Utils::Rectangle<double> getRect() override;
  void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom) override;
  bool getProperty(const std::string& name, std::string& value) override;
  bool isPropertyDefined(const std::string& name) override;
  std::string getTitle() override;

  ////////////////////////////////////////////////////////////////////////
  // PipetteViewInterface
  ////////////////////////////////////////////////////////////////////////

  PipetteLayerOperations::PipetteColor getPixelAverages(Scroom::Utils::Rectangle<int> area) override;

  ////////////////////////////////////////////////////////////////////////
  // PresentationBase
  ////////////////////////////////////////////////////////////////////////

  void                             viewAdded(ViewInterface::WeakPtr viewInterface) override;
  void                             viewRemoved(ViewInterface::WeakPtr vi) override;
  std::set<ViewInterface::WeakPtr> getViews() override;

  ////////////////////////////////////////////////////////////////////////
  // Colormappable
  ////////////////////////////////////////////////////////////////////////

  void          setColormap(Colormap::Ptr colormap) override;
  Colormap::Ptr getOriginalColormap() override;
  int           getNumberOfColors() override;
  Color         getMonochromeColor() override;
  void          setMonochromeColor(const Color& c) override;
  void          setTransparentBackground() override;
  void          disableTransparentBackground() override;
  bool          getTransparentBackground() override;
};
