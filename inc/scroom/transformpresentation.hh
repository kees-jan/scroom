/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <scroom/colormappable.hh>
#include <scroom/point.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/pipetteviewinterface.hh>
#include <scroom/rectangle.hh>

class TransformationData
{
private:
  Scroom::Utils::Point<double> aspectRatio;
  
public:
  typedef boost::shared_ptr<TransformationData> Ptr;

  static Ptr create();

  void setAspectRatio(double x, double y);
  Scroom::Utils::Point<double> getAspectRatio() const;

private:
  TransformationData();
};

class TransformPresentation : public PresentationInterface, public Colormappable, public PipetteViewInterface
{
public:
  typedef boost::shared_ptr<TransformPresentation> Ptr;

private:
  TransformationData::Ptr transformationData;
  PresentationInterface::Ptr presentation;
  Colormappable::Ptr colormappable;

private:
  TransformPresentation(PresentationInterface::Ptr const& presentation, TransformationData::Ptr const& transformationData);

public:
  static Ptr create(PresentationInterface::Ptr const& presentation, TransformationData::Ptr const& transformationData);

  // Viewable
  virtual void open(ViewInterface::WeakPtr vi);
  virtual void close(ViewInterface::WeakPtr vi);

  // PresentationInterface
  virtual Scroom::Utils::Rectangle<double> getRect();
  virtual void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();
  virtual Scroom::Utils::Point<double> getAspectRatio() const;

  // PipetteViewInterface
  virtual PipetteLayerOperations::PipetteColor getPixelAverages(Scroom::Utils::Rectangle<int> area);

  // Colormappable
  virtual void setColormap(Colormap::Ptr colormap);
  virtual Colormap::Ptr getOriginalColormap();
  virtual int getNumberOfColors();
  virtual Color getMonochromeColor();
  virtual void setMonochromeColor(const Color& c);
  virtual void setTransparentBackground();
  virtual void disableTransparentBackground();
  virtual bool getTransparentBackground();
  
};
