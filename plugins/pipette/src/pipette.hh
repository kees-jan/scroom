/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <atomic>
#include <mutex>
#include <optional>

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
  using Ptr = std::shared_ptr<PipetteHandler>;

private:
  std::optional<Selection> selection;
  bool                     enabled{false};
  std::atomic_flag         wasDisabled = ATOMIC_FLAG_INIT;
  std::mutex               jobMutex;
  ThreadPool::Queue::Ptr   currentJob{ThreadPool::Queue::createAsync()};

public:
  static Ptr create();

public:
  ////////////////////////////////////////////////////////////////////////
  // PostRenderer

  void render(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom) override;

  ////////////////////////////////////////////////////////////////////////
  // SelectionListener

  std::string getSelectionType() override;
  void        onSelectionStart(Selection p, ViewInterface::Ptr view) override;
  void        onSelectionUpdate(Selection s, ViewInterface::Ptr view) override;
  void        onSelectionEnd(Selection s, ViewInterface::Ptr view) override;

  ////////////////////////////////////////////////////////////////////////
  // ToolStateListener

  void onEnable() override;
  void onDisable() override;

  ////////////////////////////////////////////////////////////////////////

  virtual void computeValues(const ViewInterface::Ptr& view, Scroom::Utils::Rectangle<double> sel_rect);
  virtual void displayValues(const ViewInterface::Ptr&                   view,
                             Scroom::Utils::Rectangle<double>            rect,
                             const PipetteLayerOperations::PipetteColor& colors);

  ////////////////////////////////////////////////////////////////////////
  // Testing

  std::optional<Selection> getSelection() const { return selection; }
  bool                     isEnabled() const { return enabled; }
};

class Pipette
  : public PluginInformationInterface
  , public ViewObserver
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = std::shared_ptr<Pipette>;

private:
  Pipette() = default;

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
