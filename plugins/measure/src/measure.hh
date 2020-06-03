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

class MeasureHandler : public PostRenderer, public SelectionListener, virtual public Scroom::Utils::Base
{
public:
  MeasureHandler();

public:
  typedef boost::shared_ptr<MeasureHandler> Ptr;

private:
  Selection* selection;

public:
  ViewInterface::Ptr view;

public:
  static Ptr create();

public:
  virtual ~MeasureHandler();

  ////////////////////////////////////////////////////////////////////////
  // PostRenderer

  virtual void render(cairo_t* cr);

  ////////////////////////////////////////////////////////////////////////
  // SelectionListener

  virtual void onSelectionStart(GdkPoint p);
  virtual void onSelectionUpdate(Selection* s);
  virtual void onSelectionEnd(Selection* s);

  ////////////////////////////////////////////////////////////////////////

private:
  virtual void displayMeasurement();
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
