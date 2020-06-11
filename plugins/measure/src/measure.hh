/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/plugininformationinterface.hh>
#include <scroom/utilities.hh>
#include <scroom/viewinterface.hh>
#include <scroom/point.hh>

class MeasureHandler : public ToolStateListener, public PostRenderer, public SelectionListener, virtual public Scroom::Utils::Base
{
public:
  MeasureHandler();

public:
  typedef boost::shared_ptr<MeasureHandler> Ptr;

private:
  Selection::Ptr selection;
  bool enabled;

public:
  static Ptr create();

public:
  virtual ~MeasureHandler();

  ////////////////////////////////////////////////////////////////////////
  // PostRenderer

  virtual void render(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom);

  ////////////////////////////////////////////////////////////////////////
  // SelectionListener

  virtual void onSelectionStart(GdkPoint p, ViewInterface::Ptr view);
  virtual void onSelectionUpdate(Selection::Ptr s, ViewInterface::Ptr view);
  virtual void onSelectionEnd(Selection::Ptr s, ViewInterface::Ptr view);

  ////////////////////////////////////////////////////////////////////////
  // ToolStateListener

  virtual void onEnable();
  virtual void onDisable();

  ////////////////////////////////////////////////////////////////////////

private:
  virtual void displayMeasurement(ViewInterface::Ptr view);
  virtual void drawCross(cairo_t* cr, Scroom::Utils::Point<double> p);
};

class Measure : public PluginInformationInterface, public ViewObserver, virtual public Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<Measure> Ptr;

private:
  Measure() {};

public:
  static Ptr create();

public:
  ////////////////////////////////////////////////////////////////////////
  // PluginInformationInterface

  virtual std::string getPluginName();
  virtual std::string getPluginVersion();
  virtual void registerCapabilities(ScroomPluginInterface::Ptr host);

  ////////////////////////////////////////////////////////////////////////
  // ViewObserver

  virtual Scroom::Bookkeeping::Token viewAdded(ViewInterface::Ptr v);

  ////////////////////////////////////////////////////////////////////////
};
