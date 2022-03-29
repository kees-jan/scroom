/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cmath>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <cairo.h>

#include <scroom/presentationinterface.hh>
#include <scroom/scroominterface.hh>
#include <scroom/stuff.hh>
#include <scroom/utilities.hh>
#include <scroom/viewinterface.hh>

#include "../ruler/src/ruler.hh"
#include "progressbarmanager.hh"
#include "sidebarmanager.hh"

class TweakPresentationPosition;
class TweakPositionTextBox;
class TweakRulers;
class TweakSelection;

class View
  : public ViewInterface
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<View>;

private:
  GtkBuilder*                      scroomXml;
  PresentationInterface::Ptr       presentation;
  SidebarManager                   sidebarManager;
  GtkWindow*                       window;
  GtkWidget*                       menubar;
  GtkWidget*                       drawingArea;
  Scroom::Utils::Point<int>        drawingAreaSize;
  Scroom::Utils::Rectangle<double> presentationRect;
  GtkScrollbar*                    vscrollbar;
  GtkScrollbar*                    hscrollbar;
  GtkAdjustment*                   vscrollbaradjustment;
  GtkAdjustment*                   hscrollbaradjustment;
  GtkDrawingArea*                  hruler_area;
  GtkDrawingArea*                  vruler_area;
  Ruler::Ptr                       vruler;
  Ruler::Ptr                       hruler;

  GtkComboBox*                                       zoomBox;
  GtkListStore*                                      zoomItems;
  GtkProgressBar*                                    progressBar;
  GtkStatusbar*                                      statusBar;
  GtkToolbar*                                        toolBar;
  GtkToolItem*                                       toolBarSeparator;
  GtkEntry*                                          xTextBox;
  GtkEntry*                                          yTextBox;
  GtkWidget*                                         statusArea;
  GtkWidget*                                         toolbarArea;
  unsigned                                           toolBarCount;
  int                                                statusBarContextId;
  int                                                zoom;
  Scroom::Utils::Point<double>                       position; /**< of the top left visible pixel */
  Selection::Ptr                                     selection;
  Selection::Ptr                                     tweakedSelection;
  std::vector<SelectionListener::Ptr>                selectionListeners;
  std::vector<PostRenderer::Ptr>                     postRenderers;
  std::map<GtkToggleButton*, ToolStateListener::Ptr> tools;
  Scroom::Utils::Point<double>                       aspectRatio;
  boost::shared_ptr<TweakPresentationPosition>       tweakPresentationPosition;
  boost::shared_ptr<TweakPositionTextBox>            tweakPositionTextBox;
  boost::shared_ptr<TweakRulers>                     tweakRulers;
  boost::shared_ptr<TweakSelection>                  tweakSelection;

  gint                         modifiermove;
  Scroom::Utils::Point<double> cachedPoint;

  ProgressBarManager::Ptr progressBarManager;

  std::map<PresentationInterface::WeakPtr, GtkWidget*> presentations;

private:
  enum LocationChangeCause
  {
    SCROLLBAR,
    TEXTBOX,
    OTHER
  };

private:
  explicit View(GtkBuilder* scroomXml);

public:
  static Ptr create(GtkBuilder* scroomXml, PresentationInterface::Ptr presentation);

  ~View() override;
  View(const View&)           = delete;
  View(View&&)                = delete;
  View operator=(const View&) = delete;
  View operator=(View&&)      = delete;

  void redraw(cairo_t* cr);
  void hide();
  bool hasPresentation();
  void setPresentation(PresentationInterface::Ptr presentation);
  void clearPresentation();

  static void updateScrollbar(GtkAdjustment* adj,
                              int            zoom,
                              double         value,
                              double         presentationStart,
                              double         presentationSize,
                              double         windowSize);
  void        updateScrollbars();
  void        updateZoom();
  void        updateRulers();
  void        updateTextbox();
  void        toolButtonToggled(GtkToggleButton* button);

  ////////////////////////////////////////////////////////////////////////
  // Scroom events

  void on_newPresentationInterfaces_update(const std::map<NewPresentationInterface::Ptr, std::string>& newPresentationInterfaces);
  void on_presentation_created(PresentationInterface::Ptr p);
  void on_presentation_destroyed();
  void on_configure();
  void on_window_size_changed(const Scroom::Utils::Point<int>& newSize);
  void on_zoombox_changed();
  void on_textbox_value_changed(GtkEditable* editable);
  void on_scrollbar_value_changed(GtkAdjustment* adjustment);
  void on_scrollwheel(GdkEventScroll* event);
  void on_buttonPress(GdkEventButton* event);
  void on_buttonRelease(GdkEventButton* event);
  void on_motion_notify(GdkEventMotion* event);
  void setFullScreen();
  void unsetFullScreen();

  ////////////////////////////////////////////////////////////////////////
  // ViewInterface

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
  void                       addToolButton(GtkToggleButton*, ToolStateListener::Ptr) override;

  ////////////////////////////////////////////////////////////////////////
  // Helpers

private:
  Scroom::Utils::Point<double> windowPointToPresentationPoint(Scroom::Utils::Point<double> wp) const;
  Scroom::Utils::Point<double> presentationPointToWindowPoint(Scroom::Utils::Point<double> presentationpoint) const;
  Scroom::Utils::Point<double> tweakedPosition() const;
  void                         updateNewWindowMenu();
  void                         on_zoombox_changed(int newzoom, const Scroom::Utils::Point<double>& mousePos);
  void                         updateXY(const Scroom::Utils::Point<double>& newPos, const LocationChangeCause& source);
};
