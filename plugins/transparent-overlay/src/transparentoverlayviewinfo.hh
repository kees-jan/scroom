/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <gtk/gtk.h>

#include <cairo.h>

#include <scroom/presentationinterface.hh>
#include <scroom/progressinterfacehelpers.hh>
#include <scroom/utilities.hh>
#include <scroom/viewinterface.hh>

#include "sizedeterminer.hh"

class TransparentOverlayViewInfo;

class ChildView
  : public ViewInterface
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<ChildView>;

private:
  boost::shared_ptr<TransparentOverlayViewInfo> parent;
  ProgressInterface::Ptr                        progressInterface;

private:
  explicit ChildView(boost::shared_ptr<TransparentOverlayViewInfo> parent);

public:
  static Ptr create(boost::shared_ptr<TransparentOverlayViewInfo> const& parent);

  // ViewInterface ///////////////////////////////////////////////////////
  void                       invalidate() override;
  ProgressInterface::Ptr     getProgressInterface() override;
  void                       addSideWidget(std::string title, GtkWidget* w) override;
  void                       removeSideWidget(GtkWidget* w) override;
  void                       addToToolbar(GtkToolItem* ti) override;
  void                       removeFromToolbar(GtkToolItem* ti) override;
  void                       registerSelectionListener(SelectionListener::Ptr listener) override;
  void                       registerPostRenderer(PostRenderer::Ptr renderer) override;
  void                       setStatusMessage(const std::string& message) override;
  PresentationInterface::Ptr getCurrentPresentation() override;
  void                       addToolButton(GtkToggleButton* name, ToolStateListener::Ptr callback) override;
};

class TransparentOverlayViewInfo : virtual public Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<TransparentOverlayViewInfo>;

private:
  using ChildMap = std::map<PresentationInterface::Ptr, ChildView::Ptr>;

private:
  ViewInterface::Ptr                               parentView;
  ChildMap                                         childViews;
  Scroom::Utils::ProgressInterfaceMultiplexer::Ptr progressInterfaceMultiplexer;
  std::vector<GtkWidget*>                          buttons;
  std::vector<PresentationInterface::Ptr>          children;
  SizeDeterminer::Ptr                              sizeDeterminer;

private:
  TransparentOverlayViewInfo(const ViewInterface::WeakPtr& vi, SizeDeterminer::Ptr sizeDeterminer);
  void createToggleToolButton(PresentationInterface::Ptr const& p);

public:
  static Ptr                 create(const ViewInterface::WeakPtr& vi, SizeDeterminer::Ptr const& sizeDeterminer);
  void                       addChildren(const std::list<PresentationInterface::Ptr>& children);
  void                       addChild(const PresentationInterface::Ptr& child);
  PresentationInterface::Ptr getChild(const ChildView::Ptr& cv);

  void close();

  void redraw(cairo_t* cr, Scroom::Utils::Rectangle<double> const& presentationArea, int zoom);

  void                   invalidate() { parentView->invalidate(); }
  ProgressInterface::Ptr getProgressInterface() { return progressInterfaceMultiplexer->createProgressInterface(); }

  // Helpers
  void toggled(GtkToggleButton* button);
};
