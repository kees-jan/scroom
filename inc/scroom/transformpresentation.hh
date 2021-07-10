/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <scroom/colormappable.hh>
#include <scroom/pipetteviewinterface.hh>
#include <scroom/point.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/rectangle.hh>

class TransformationData
{
private:
  Scroom::Utils::Point<double> aspectRatio;

public:
  using Ptr = boost::shared_ptr<TransformationData>;

  static Ptr create(Scroom::Utils::Point<double> aspectRatio_);
  static Ptr create();

  void                                       setAspectRatio(double x, double y);
  void                                       setAspectRatio(Scroom::Utils::Point<double> aspectRatio_);
  [[nodiscard]] Scroom::Utils::Point<double> getAspectRatio() const;

private:
  TransformationData();
  explicit TransformationData(Scroom::Utils::Point<double> aspectRatio_);
};

class TransformPresentation
  : public PresentationBaseSimple
  , public Colormappable
  , public PipetteViewInterface
{
public:
  using Ptr = boost::shared_ptr<TransformPresentation>;

private:
  TransformationData::Ptr    transformationData;
  PresentationInterface::Ptr presentation;
  Colormappable::Ptr         colormappable;

private:
  TransformPresentation(PresentationInterface::Ptr const& presentation, TransformationData::Ptr const& transformationData);

public:
  static Ptr create(PresentationInterface::Ptr const& presentation, TransformationData::Ptr const& transformationData);

  // Viewable
  void open(ViewInterface::WeakPtr vi) override;
  void close(ViewInterface::WeakPtr vi) override;

  // PresentationInterface
  Scroom::Utils::Rectangle<double> getRect() override;
  void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom) override;
  bool getProperty(const std::string& name, std::string& value) override;
  bool isPropertyDefined(const std::string& name) override;
  std::string                  getTitle() override;
  Scroom::Utils::Point<double> getAspectRatio() const override;

  // PipetteViewInterface
  PipetteLayerOperations::PipetteColor getPixelAverages(Scroom::Utils::Rectangle<int> area) override;

  // Colormappable
  void          setColormap(Colormap::Ptr colormap) override;
  Colormap::Ptr getOriginalColormap() override;
  int           getNumberOfColors() override;
  Color         getMonochromeColor() override;
  void          setMonochromeColor(const Color& c) override;
  void          setTransparentBackground() override;
  void          disableTransparentBackground() override;
  bool          getTransparentBackground() override;
  void          showMetadata() override;
};
