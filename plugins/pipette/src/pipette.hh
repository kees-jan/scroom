/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <atomic>
#include <mutex>

#include <scroom/pipetteviewinterface.hh>
#include <scroom/plugininformationinterface.hh>
#include <scroom/threadpool.hh>
#include <scroom/utilities.hh>
#include <scroom/viewinterface.hh>

class PipetteHandler
  : public ToolStateListener
  , public PostRenderer
  , public SelectionListener
  , virtual public Scroom::Utils::Base
{
public:
  PipetteHandler();

public:
  typedef boost::shared_ptr<PipetteHandler> Ptr;

private:
  Selection::Ptr         selection;
  bool                   enabled;
  std::atomic_flag       wasDisabled;
  std::mutex             jobMutex;
  ThreadPool::Queue::Ptr currentJob;

public:
  static Ptr create();

public:
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

  virtual void computeValues(ViewInterface::Ptr view, Scroom::Utils::Rectangle<int> sel_rect);
  virtual void
    displayValues(ViewInterface::Ptr view, Scroom::Utils::Rectangle<int> rect, PipetteLayerOperations::PipetteColor colors);

  ////////////////////////////////////////////////////////////////////////
  // Testing

  Selection::ConstPtr getSelection() const { return selection; }
  bool                isEnabled() const { return enabled; }
};

class Pipette
  : public PluginInformationInterface
  , public ViewObserver
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<Pipette>;

private:
  Pipette(){};

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
