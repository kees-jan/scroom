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

class MeasureListener : public SelectionListener, virtual public Scroom::Utils::Base{
public:
  MeasureListener();

public:
  typedef boost::shared_ptr<MeasureListener> Ptr;

public:
  static Ptr create();

public:
  virtual ~MeasureListener();

  virtual void onSelection(Selection* measurement);
};

class MeasureRenderer : public PostRenderer, virtual public Scroom::Utils::Base{
public:
	MeasureRenderer();

public:
  typedef boost::shared_ptr<MeasureRenderer> Ptr;

public:
  static Ptr create();

public:
  virtual ~MeasureRenderer();

  virtual void render(cairo_t* cr);
};

class Measure : public PluginInformationInterface, public ViewObserver, virtual public Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<Measure> Ptr;

private:
  Measure();

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

  virtual ~Measure();
};

