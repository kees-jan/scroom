/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
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

#include "gtkruler.h"
#include "progressbarmanager.hh"
#include "sidebarmanager.hh"

class View
  : public ViewInterface
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<View>;

private:
  GtkBuilder*                                        scroomXml;
  PresentationInterface::Ptr                         presentation;
  SidebarManager                                     sidebarManager;
  GtkWindow*                                         window;
  GtkWidget*                                         menubar;
  GtkWidget*                                         drawingArea;
  int                                                drawingAreaWidth;
  int                                                drawingAreaHeight;
  Scroom::Utils::Rectangle<double>                   presentationRect;
  GtkScrollbar*                                      vscrollbar;
  GtkScrollbar*                                      hscrollbar;
  GtkAdjustment*                                     vscrollbaradjustment;
  GtkAdjustment*                                     hscrollbaradjustment;
  GtkRuler*                                          hruler;
  GtkRuler*                                          vruler;
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
  int                                                x; /**< x-coordinate of the top left visible pixel */
  int                                                y; /**< y-coordinate of the top left visible pixel */
  Selection::Ptr                                     selection;
  std::vector<SelectionListener::Ptr>                selectionListeners;
  std::vector<PostRenderer::Ptr>                     postRenderers;
  std::map<GtkToggleButton*, ToolStateListener::Ptr> tools;
  Scroom::Utils::Point<double>                       aspectRatio;

  gint     modifiermove;
  GdkPoint cachedPoint{0, 0};

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
  View(GtkBuilder* scroomXml);

public:
  static Ptr create(GtkBuilder* scroomXml, PresentationInterface::Ptr presentation);

  ~View() override;
  View(const View&) = delete;
  View(View&&)      = delete;
  View operator=(const View&) = delete;
  View operator=(View&&) = delete;


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
  void on_window_size_changed(int newWidth, int newHeight);
  void on_zoombox_changed();
  void on_zoombox_changed(int newZoom, int mousex, int mousey);
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
  GdkPoint        windowPointToPresentationPoint(GdkPoint wp) const;
  GdkPoint        presentationPointToWindowPoint(GdkPoint presentationpoint) const;
  static GdkPoint eventToPoint(GdkEventButton* event);
  static GdkPoint eventToPoint(GdkEventMotion* event);
  void            updateNewWindowMenu();
  void            updateXY(int x, int y, LocationChangeCause source);
};
