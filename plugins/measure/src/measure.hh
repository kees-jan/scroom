/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/plugininformationinterface.hh>
#include <scroom/point.hh>
#include <scroom/utilities.hh>
#include <scroom/viewinterface.hh>

class MeasureHandler
  : public ToolStateListener
  , public PostRenderer
  , public SelectionListener
  , virtual public Scroom::Utils::Base
{
public:
  MeasureHandler();

public:
  using Ptr = boost::shared_ptr<MeasureHandler>;

private:
  Selection::Ptr selection;
  bool           enabled{false};

public:
  static Ptr create();

  ////////////////////////////////////////////////////////////////////////
  // PostRenderer

  void render(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom) override;

  ////////////////////////////////////////////////////////////////////////
  // SelectionListener

  void onSelectionStart(GdkPoint p, ViewInterface::Ptr view) override;
  void onSelectionUpdate(Selection::Ptr s, ViewInterface::Ptr view) override;
  void onSelectionEnd(Selection::Ptr s, ViewInterface::Ptr view) override;

  ////////////////////////////////////////////////////////////////////////
  // ToolStateListener

  void onEnable() override;
  void onDisable() override;

  ////////////////////////////////////////////////////////////////////////

private:
  virtual void displayMeasurement(ViewInterface::Ptr view);
  virtual void drawCross(cairo_t* cr, Scroom::Utils::Point<double> p);
};

class Measure
  : public PluginInformationInterface
  , public ViewObserver
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<Measure>;

private:
  Measure() = default;

public:
  static Ptr create();

public:
  ////////////////////////////////////////////////////////////////////////
  // PluginInformationInterface

  std::string getPluginName() override;
  std::string getPluginVersion() override;
  void        registerCapabilities(ScroomPluginInterface::Ptr host) override;

  ////////////////////////////////////////////////////////////////////////
  // ViewObserver

  Scroom::Bookkeeping::Token viewAdded(ViewInterface::Ptr v) override;

  ////////////////////////////////////////////////////////////////////////
};
