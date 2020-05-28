/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <stdlib.h>

#include <map>
#include <vector>
#include <string>
#include <cmath>

#include <glade/glade.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <cairo.h>

#include <scroom/scroominterface.hh>
#include <scroom/viewinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/utilities.hh>
#include <scroom/stuff.hh>

#include "sidebarmanager.hh"
#include "progressbarmanager.hh"

class View : public ViewInterface, virtual public Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<View> Ptr;

private:
  GladeXML* scroomXml;
  PresentationInterface::Ptr presentation;
  SidebarManager sidebarManager;
  GtkWindow* window;
  GtkWidget* menubar;
  GtkWidget* drawingArea;
  int drawingAreaWidth;
  int drawingAreaHeight;
  Scroom::Utils::Rectangle<double> presentationRect;
  GtkVScrollbar* vscrollbar;
  GtkHScrollbar* hscrollbar;
  GtkAdjustment* vscrollbaradjustment;
  GtkAdjustment* hscrollbaradjustment;
  GtkRuler* hruler;
  GtkRuler* vruler;
  GtkComboBox* zoomBox;
  GtkListStore* zoomItems;
  GtkProgressBar* progressBar;
  GtkStatusbar* statusBar;
  GtkToolbar* toolBar;
  GtkToolItem* toolBarSeparator;
  GtkEntry* xTextBox;
  GtkEntry* yTextBox;
  GtkWidget* statusArea;
  GtkWidget* toolbarArea;
  unsigned toolBarCount;
  int statusBarContextId;
  int zoom;
  int x;
  int y;
  std::map<guint, Selection*> selections;
  std::map<MouseButton, std::vector<SelectionListener::Ptr>> selectionListeners;
  std::vector<PostRenderer::Ptr> postRenderers;

  gint modifiermove;
  GdkPoint cachedPoint;
  bool panning;

  ProgressBarManager::Ptr progressBarManager;

  std::map<PresentationInterface::WeakPtr,GtkWidget*> presentations;

private:
  enum LocationChangeCause
    {
      SCROLLBAR,
      TEXTBOX,
      OTHER
    };

private:
  View(GladeXML* scroomXml);

public:
  static Ptr create(GladeXML* scroomXml, PresentationInterface::Ptr presentation);

  virtual ~View();

  void redraw(cairo_t* cr);
  void hide();
  bool hasPresentation();
  void setPresentation(PresentationInterface::Ptr presentation);
  void clearPresentation();

  void updateScrollbar(GtkAdjustment* adj, int zoom, double value,
                       double presentationStart, double presentationSize, double windowSize);
  void updateScrollbars();
  void updateZoom();
  void updateRulers();
  void updateTextbox();

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

  virtual void invalidate();
  virtual ProgressInterface::Ptr getProgressInterface();
  virtual void addSideWidget(std::string title, GtkWidget* w);
  virtual void removeSideWidget(GtkWidget* w);
  virtual void addToToolbar(GtkToolItem* ti);
  virtual void removeFromToolbar(GtkToolItem* ti);
  virtual void setPanning();
  virtual void unsetPanning();
  virtual void registerSelectionListener(SelectionListener::Ptr listener, MouseButton button);
  virtual void registerPostRenderer(PostRenderer::Ptr renderer);
  virtual void setStatusMessage(const std::string& message);
  virtual GdkPoint presentationPointToWindowPoint(GdkPoint presentationpoint);
  virtual Scroom::Utils::Stuff getCurrentPresentation();

  ////////////////////////////////////////////////////////////////////////
  // Helpers

private:
  GdkPoint windowPointToPresentationPoint(GdkPoint wp);
  GdkPoint eventToPoint(GdkEventButton* event);
  GdkPoint eventToPoint(GdkEventMotion* event);
  void drawCross(cairo_t* cr, GdkPoint p);
  void updateNewWindowMenu();
  void updateXY(int x, int y, LocationChangeCause source);
};

