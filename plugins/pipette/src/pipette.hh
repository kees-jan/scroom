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
#include <scroom/threadpool.hh>
#include <scroom/pipetteviewinterface.hh>
#include <mutex>
#include <atomic>

class PipetteHandler : public ToolStateListener, public PostRenderer, public SelectionListener, virtual public Scroom::Utils::Base
{
public:
  PipetteHandler();

public:
  typedef boost::shared_ptr<PipetteHandler> Ptr;

private:
  Selection::Ptr selection;
  bool enabled;
  std::atomic_flag wasDisabled;
  std::mutex jobMutex;
  ThreadPool::Queue::Ptr currentJob;

public:
  static Ptr create();

public:
  virtual ~PipetteHandler();

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

  virtual void computeValues(ViewInterface::Ptr view, Scroom::Utils::Rectangle<int> sel_rect);
  virtual void displayValues(ViewInterface::Ptr view, Scroom::Utils::Rectangle<int> rect, PipetteLayerOperations::PipetteColor colors);
};

class Pipette : public PluginInformationInterface, public ViewObserver, virtual public  Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<Pipette> Ptr;

private:
  Pipette() {};

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
