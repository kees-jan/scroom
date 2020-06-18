/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "view.hh"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cmath>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include <glib-object.h>

#include "pluginmanager.hh"
#include "callbacks.hh"

#ifndef G_VALUE_INIT
#define G_VALUE_INIT {0,{{0}}}
#endif

static const char *zoomfactor[] =
  {
    "32:1",
    "16:1",
    "8:1",
    "4:1",
    "2:1",
    "1:1",
    "1:2",
    "1:4",
    "1:8",
    "1:16",
    "1:32",
    "1:64",
    "1:128",
    "1:250",
    "1:500",
    "1:1000",
    "1:2000",
    "1:4000",
    "1:8000",
    "1:16000",
    "1:32000",
    "1:64000",
    "1:128000",
    "1:250000",
    "1:500000",
    "1:1 million",
    "1:2 million",
    "1:4 million",
    "1:8 million",
    "1:16 million",
    "1:32 million",
    "1:64 million",
    "1:128 million",
    "1:250 million",
    "1:500 million",
    "1:1 billion",
  };
static const int MaxZoom=5;

enum
  {
    COLUMN_TEXT,
    COLUMN_ZOOM,
    N_COLUMNS
  };

////////////////////////////////////////////////////////////////////////
/// Helpers

// This one has too much View-internal knowledge to hide in callbacks.cc
static void on_newWindow_activate(GtkMenuItem*, gpointer user_data)
{
  PresentationInterface::WeakPtr& wp = *static_cast<PresentationInterface::WeakPtr*>(user_data); // Yuk!
  PresentationInterface::Ptr p = wp.lock();
  if(p)
  {
    find_or_create_scroom(p);
  }
}

////////////////////////////////////////////////////////////////////////

View::View(GladeXML* scroomXml_)
  : scroomXml(scroomXml_), presentation(), sidebarManager(),
    drawingAreaWidth(0), drawingAreaHeight(0),
    zoom(0), x(0), y(0), aspectRatio(Scroom::Utils::make_point(1.0, 1.0)), modifiermove(0)
{
  PluginManager::Ptr pluginManager = PluginManager::getInstance();
  window = GTK_WINDOW(glade_xml_get_widget(scroomXml_, "scroom"));
  drawingArea = glade_xml_get_widget(scroomXml_, "drawingarea");
  vscrollbar = GTK_VSCROLLBAR(glade_xml_get_widget(scroomXml_, "vscrollbar"));
  hscrollbar = GTK_HSCROLLBAR(glade_xml_get_widget(scroomXml_, "hscrollbar"));
  vscrollbaradjustment = gtk_range_get_adjustment(GTK_RANGE(vscrollbar));
  hscrollbaradjustment = gtk_range_get_adjustment(GTK_RANGE(hscrollbar));
  vruler = GTK_RULER(glade_xml_get_widget(scroomXml_, "vruler"));
  hruler = GTK_RULER(glade_xml_get_widget(scroomXml_, "hruler"));
  xTextBox = GTK_ENTRY(glade_xml_get_widget(scroomXml_, "x_textbox"));
  yTextBox = GTK_ENTRY(glade_xml_get_widget(scroomXml_, "y_textbox"));

  menubar = GTK_WIDGET(glade_xml_get_widget(scroomXml_, "menubar"));
  statusArea = GTK_WIDGET(glade_xml_get_widget(scroomXml_, "status_area"));
  toolbarArea = GTK_WIDGET(glade_xml_get_widget(scroomXml_, "toolbar_area"));

  zoomBox = GTK_COMBO_BOX(glade_xml_get_widget(scroomXml_, "zoomboxcombo"));
  zoomItems = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_INT);

  gtk_combo_box_set_model(zoomBox, GTK_TREE_MODEL(zoomItems));
  GtkCellRenderer* txt = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
  gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(zoomBox),
                           txt,
                           true);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(zoomBox), txt,
                                 "text", COLUMN_TEXT,
                                 NULL);

  progressBar = GTK_PROGRESS_BAR(glade_xml_get_widget(scroomXml_, "progressbar"));
  progressBarManager = ProgressBarManager::create(progressBar);
  statusBar = GTK_STATUSBAR(glade_xml_get_widget(scroomXml_, "statusbar"));
  statusBarContextId = gtk_statusbar_get_context_id(statusBar, "View");

  GtkWidget* panelWindow = glade_xml_get_widget(scroomXml_, "panelWindow");
  GtkBox* panel = GTK_BOX(glade_xml_get_widget(scroomXml_, "panel"));
  sidebarManager.setWidgets(panelWindow, panel);
  toolBar = GTK_TOOLBAR(glade_xml_get_widget(scroomXml_, "toolbar"));
  toolBarSeparator = NULL;
  toolBarCount = 0;

  cachedPoint.x=0;
  cachedPoint.y=0;

  on_newPresentationInterfaces_update(pluginManager->getNewPresentationInterfaces());
  updateNewWindowMenu();
  on_configure();
}

View::Ptr View::create(GladeXML* scroomXml, PresentationInterface::Ptr presentation)
{
  Ptr view(new View(scroomXml));
  printf("Creating a new view\n");

  if(presentation)
  {
    view->setPresentation(presentation);
  }

  return view;
}

View::~View()
{
  printf("Destroying view...\n");
  gtk_widget_destroy(GTK_WIDGET(window));
}

void View::redraw(cairo_t* cr)
{
  if(presentation)
  {
    GdkRectangle rect;
    rect.x=x;
    rect.y=y;
    if(zoom>=0)
    {
      // Zooming in. Smallest step is 1 presentation pixel, which is more than one window-pixel
      int pixelSize = 1<<zoom;
      rect.width = (drawingAreaWidth+pixelSize-1)/pixelSize;
      rect.height = (drawingAreaHeight+pixelSize-1)/pixelSize;
    }
    else
    {
      // Zooming out. Smallest step is 1 window-pixel, which is more than one presentation-pixel
      int pixelSize = 1<<(-zoom);
      rect.width = drawingAreaWidth*pixelSize;
      rect.height = drawingAreaHeight*pixelSize;

      // Round to whole pixels
      rect.x = (rect.x/pixelSize) * pixelSize;
      rect.y = (rect.y/pixelSize) * pixelSize;
    }

    presentation->redraw(shared_from_this<View>(), cr, rect, zoom);

    for(auto renderer : postRenderers)
    {
      renderer->render(shared_from_this<View>(), cr, rect, zoom);
    }
  }
  else
  {
    // A logo here would be nice...

    // char buffer[] = "View says \"Hi\"";
    //
    // cairo_move_to(cr, 50, 50);
    // cairo_show_text(cr, buffer);
  }
}

void View::hide()
{
  gtk_widget_hide(GTK_WIDGET(window));
}

bool View::hasPresentation()
{
  return presentation!=NULL;
}

void View::clearPresentation()
{
  setPresentation(PresentationInterface::Ptr()); // null
}

void View::setPresentation(PresentationInterface::Ptr presentation_)
{
  View::Ptr me = shared_from_this<View>();

  if(this->presentation)
  {
    this->presentation->close(me);
    this->presentation.reset();
  }

  this->presentation = presentation_;

  if(this->presentation)
  {
    presentation_->open(me);
    presentationRect = presentation_->getRect();
    aspectRatio = presentation_->getAspectRatio();
    std::string s = presentation_->getTitle();
    if(s.length())
      s = "Scroom - " + s;
    else
      s = "Scroom";
    gtk_window_set_title(window, s.c_str());
  }
  updateZoom();
  updateScrollbars();
  updateTextbox();
  invalidate();
}

void View::updateScrollbar(GtkAdjustment* adj, int zoom_, double value, double presentationStart, double presentationSize, double windowSize)
{
  if(zoom_>=0)
  {
    // Zooming in. Smallest step is 1 presentation pixel, which is more than one window-pixel
    int pixelSize = 1<<zoom_;
    presentationStart -= windowSize/pixelSize/2;
    presentationSize += windowSize/pixelSize;

    gtk_adjustment_configure(adj, value, presentationStart, presentationStart+presentationSize,
                             1, 3*windowSize/pixelSize/4, windowSize/pixelSize);
  }
  else
  {
    // Zooming out. Smallest step is 1 window-pixel, which is more than one presentation-pixel
    int pixelSize = 1<<(-zoom_);
    presentationStart -= windowSize*pixelSize/2;
    presentationSize += windowSize*pixelSize;

    gtk_adjustment_configure(adj, value, presentationStart, presentationStart+presentationSize,
                             pixelSize, 3*windowSize*pixelSize/4, windowSize*pixelSize);
  }
}

void View::updateScrollbars()
{
  if(presentation)
  {
    gtk_widget_set_sensitive(GTK_WIDGET(vscrollbar), true);
    gtk_widget_set_sensitive(GTK_WIDGET(hscrollbar), true);

    updateScrollbar(hscrollbaradjustment, zoom, x,
                    presentationRect.x(), presentationRect.width(), drawingAreaWidth);
    updateScrollbar(vscrollbaradjustment, zoom, y,
                    presentationRect.y(), presentationRect.height(), drawingAreaHeight);
    updateRulers();
  }
  else
  {
    gtk_widget_set_sensitive(GTK_WIDGET(vscrollbar), false);
    gtk_widget_set_sensitive(GTK_WIDGET(hscrollbar), false);
  }
}

void View::updateTextbox()
{
  if(presentation)
  {
    gtk_widget_set_sensitive(GTK_WIDGET(xTextBox), true);
    gtk_widget_set_sensitive(GTK_WIDGET(yTextBox), true);

    int daw = drawingAreaWidth;
    int dah = drawingAreaHeight;
    if(zoom>=0)
    {
      // Zooming in. Smallest step is 1 presentation pixel, which is more than one window-pixel
      int pixelSize = 1<<zoom;
      daw /= pixelSize*aspectRatio.x;
      dah /= pixelSize*aspectRatio.y;
    }
    else
    {
      // Zooming out. Smallest step is 1 window-pixel, which is more than one presentation-pixel
      int pixelSize = 1<<(-zoom);
      daw *= pixelSize*aspectRatio.x;
      dah *= pixelSize*aspectRatio.y;
    }

    std::string xs = boost::lexical_cast<std::string>(x+daw/2);
    std::string ys = boost::lexical_cast<std::string>(y+dah/2);
    gtk_entry_set_text(xTextBox, xs.c_str());
    gtk_entry_set_text(yTextBox, ys.c_str());
  }
  else
  {
    gtk_widget_set_sensitive(GTK_WIDGET(xTextBox), false);
    gtk_widget_set_sensitive(GTK_WIDGET(yTextBox), false);
  }
}

void View::updateZoom()
{
  if(presentation)
  {
    int presentationHeight = presentationRect.height();
    int presentationWidth = presentationRect.width();
    int minZoom = 0;

    while(presentationHeight > drawingAreaHeight/2 || presentationWidth > drawingAreaWidth/2)
    {
      presentationHeight >>= 1;
      presentationWidth >>= 1;
      minZoom--;
    }

    gtk_widget_set_sensitive(GTK_WIDGET(zoomBox), true);

    int zMax = MaxZoom - minZoom;
    zMax = std::max(zMax, 1+MaxZoom-zoom);
    zMax = std::min((size_t)zMax, sizeof(zoomfactor)/sizeof(zoomfactor[0]));

    gtk_list_store_clear(zoomItems);
    for(int z=0; z<zMax; z++)
    {
      GtkTreeIter iter;
      gtk_list_store_insert_with_values(zoomItems, &iter, z,
                                        COLUMN_TEXT, zoomfactor[z],
                                        COLUMN_ZOOM, MaxZoom-z,
                                        -1);

      if(zoom == MaxZoom-z)
      {
        gtk_combo_box_set_active_iter(zoomBox, &iter);
      }
    }
  }
  else
  {
    gtk_widget_set_sensitive(GTK_WIDGET(zoomBox), false);
    gtk_list_store_clear(zoomItems);
    GtkTreeIter iter;
    gtk_list_store_insert_with_values(zoomItems, &iter, 0,
                                        COLUMN_TEXT, zoomfactor[0],
                                        COLUMN_ZOOM, MaxZoom,
                                        -1);

  }
}

void View::updateRulers()
{
  if(zoom>=0)
  {
    // Zooming in. Smallest step is 1 presentation pixel, which is more than one window-pixel
    int pixelSize = 1<<zoom;
    gtk_ruler_set_range(hruler, x, x + 1.0*drawingAreaWidth/(pixelSize*aspectRatio.x), 0, presentationRect.x() + presentationRect.width());
    gtk_ruler_set_range(vruler, y, y + 1.0*drawingAreaHeight/(pixelSize*aspectRatio.y), 0, presentationRect.y() + presentationRect.height());
  }
  else
  {
    // Zooming out. Smallest step is 1 window-pixel, which is more than one presentation-pixel
    int pixelSize = 1<<(-zoom);
    gtk_ruler_set_range(hruler, x, x + drawingAreaWidth*pixelSize*aspectRatio.x, 0, presentationRect.x() + presentationRect.width());
    gtk_ruler_set_range(vruler, y, y + drawingAreaHeight*pixelSize*aspectRatio.y, 0, presentationRect.y() + presentationRect.height());
  }
}

void View::toolButtonToggled(GtkToggleButton* button)
{
  if(gtk_toggle_button_get_active(button))
  {
    for(auto tool : tools)
    {
      if(tool.first != button && gtk_toggle_button_get_active(tool.first))
      {
        gtk_toggle_button_set_active(tool.first, false);
        gtk_widget_set_sensitive(GTK_WIDGET(tool.first), true);
        tools[tool.first]->onDisable();
      }
    }
    gtk_widget_set_sensitive(GTK_WIDGET(button), false);
    tools[button]->onEnable();
  }
}

////////////////////////////////////////////////////////////////////////
// Scroom events

void View::on_newPresentationInterfaces_update(const std::map<NewPresentationInterface::Ptr, std::string>& newPresentationInterfaces)
{
  GtkWidget* new_menu_item = glade_xml_get_widget(scroomXml, "new");

  if(newPresentationInterfaces.empty())
  {
    gtk_widget_set_sensitive(new_menu_item, false);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (new_menu_item), NULL);
  }
  else
  {
    gtk_widget_set_sensitive(new_menu_item, true);

    GtkWidget* new_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (new_menu_item), new_menu);

    for(std::map<NewPresentationInterface::Ptr, std::string>::const_iterator cur=newPresentationInterfaces.begin();
        cur != newPresentationInterfaces.end();
        cur++)
    {
      GtkWidget* menu_item = gtk_menu_item_new_with_label(cur->second.c_str());
      gtk_widget_show (menu_item);
      gtk_container_add (GTK_CONTAINER (new_menu), menu_item);

      g_signal_connect (static_cast<gpointer>(menu_item), "activate", G_CALLBACK (on_new_activate), cur->first.get());
    }
  }
}

void View::on_presentation_created(PresentationInterface::Ptr p)
{
  presentations[p]=NULL;
  updateNewWindowMenu();
}

void View::on_presentation_destroyed()
{
  updateNewWindowMenu();
}

void View::on_configure()
{
  // There should be a simpler way to do this...
  GdkRegion* r = gdk_drawable_get_visible_region(GDK_DRAWABLE(gtk_widget_get_window(drawingArea)));
  GdkRectangle rect;
  gdk_region_get_clipbox(r, &rect);

  int newWidth = rect.width;
  int newHeight = rect.height;

  if(drawingAreaHeight != newHeight || drawingAreaWidth != newWidth)
    on_window_size_changed(newWidth, newHeight);

  gdk_region_destroy(r);
}

void View::on_window_size_changed(int newWidth, int newHeight)
{
  if(zoom>=0)
  {
    const int pixelSize = 1<<zoom;
    x+=(drawingAreaWidth-newWidth)/(pixelSize*aspectRatio.x)/2;
    y+=(drawingAreaHeight-newHeight)/(pixelSize*aspectRatio.y)/2;
  }
  else
  {
    const int pixelSize = 1<<-zoom;
    x+=(drawingAreaWidth-newWidth)*(pixelSize*aspectRatio.x)/2;
    y+=(drawingAreaHeight-newHeight)*(pixelSize*aspectRatio.y)/2;
  }

  drawingAreaHeight = newHeight;
  drawingAreaWidth = newWidth;
  updateZoom();
  updateScrollbars();
  invalidate();
}

void View::on_scrollwheel(GdkEventScroll* event)
{
  if(event->direction == GDK_SCROLL_UP ||
     event->direction == GDK_SCROLL_DOWN)
  {
    int newZoom = zoom + ((event->direction == GDK_SCROLL_UP)?1:-1);
    newZoom = std::min(MaxZoom, newZoom);

    GtkTreeIter iter;
    for(bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(zoomItems), &iter);
        valid;
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(zoomItems), &iter))
    {
      GValue value= G_VALUE_INIT;
      gtk_tree_model_get_value(GTK_TREE_MODEL(zoomItems), &iter, COLUMN_ZOOM, &value);
      int foundZoom = g_value_get_int(&value);

      if(foundZoom==newZoom)
      {
        on_zoombox_changed(newZoom, event->x, event->y);
        gtk_combo_box_set_active_iter(zoomBox, &iter);
        break;
      }
    }
  }
}

void View::on_zoombox_changed()
{
  GtkTreeIter iter;
  GValue value= G_VALUE_INIT;
  gtk_combo_box_get_active_iter(zoomBox, &iter);

  if(gtk_list_store_iter_is_valid(zoomItems, &iter))
  {
    gtk_tree_model_get_value(GTK_TREE_MODEL(zoomItems), &iter, COLUMN_ZOOM, &value);
    int newZoom = g_value_get_int(&value);
    on_zoombox_changed(newZoom, drawingAreaWidth/2, drawingAreaHeight/2);
  }
}

void View::on_zoombox_changed(int newzoom, int mousex, int mousey)
{
  if(newzoom!=zoom)
  {
    if(zoom>=0)
    {
      const int pixelsize = 1<<zoom;
      x+=mousex/(pixelsize*aspectRatio.x);
      y+=mousey/(pixelsize*aspectRatio.y);
    }
    else
    {
      const int pixelsize = 1<<-zoom;
      x+=mousex*pixelsize*aspectRatio.x;
      y+=mousey*pixelsize*aspectRatio.y;
    }

    if(newzoom>=0)
    {
      const int pixelsize = 1<<newzoom;
      x-=mousex/(pixelsize*aspectRatio.x);
      y-=mousey/(pixelsize*aspectRatio.y);
    }
    else
    {
      const int pixelsize = 1<<-newzoom;
      x-=mousex*pixelsize*aspectRatio.x;
      y-=mousey*pixelsize*aspectRatio.y;
    }

    zoom = newzoom;
    updateScrollbars();
    updateTextbox();
    invalidate();
  }
}

void View::on_textbox_value_changed(GtkEditable* editable)
{
  try
  {
    int newx = x;
    int newy = y;

    int daw = drawingAreaWidth;
    int dah = drawingAreaHeight;
    if(zoom>=0)
    {
      // Zooming in. Smallest step is 1 presentation pixel, which is more than one window-pixel
      int pixelSize = 1<<zoom;
      daw /= pixelSize*aspectRatio.x;
      dah /= pixelSize*aspectRatio.y;
    }
    else
    {
      // Zooming out. Smallest step is 1 window-pixel, which is more than one presentation-pixel
      int pixelSize = 1<<(-zoom);
      daw *= pixelSize * aspectRatio.x;
      dah *= pixelSize * aspectRatio.y;
    }

    if(editable == GTK_EDITABLE(yTextBox))
    {
      newy = boost::lexical_cast<int>(gtk_entry_get_text(yTextBox)) - dah/2;
    }
    else
    {
      newx = boost::lexical_cast<int>(gtk_entry_get_text(xTextBox)) - daw/2;
    }

    updateXY(newx, newy, TEXTBOX);
  }
  catch (boost::bad_lexical_cast& ex)
  {
    // User typed something invalid, probably a letter. Ignore...
  }
}

void View::on_scrollbar_value_changed(GtkAdjustment* adjustment)
{
  int newx = x;
  int newy = y;

  if(adjustment == vscrollbaradjustment)
  {
    newy = static_cast<int>(gtk_adjustment_get_value(adjustment));
  }
  else
  {
    newx = static_cast<int>(gtk_adjustment_get_value(adjustment));
  }

  updateXY(newx, newy, SCROLLBAR);
}

void View::on_buttonPress(GdkEventButton* event)
{
  if(event->button==1 && modifiermove==0)
  {
    // Begin left-dragging
    modifiermove = GDK_BUTTON1_MASK;
    cachedPoint = eventToPoint(event);
  }
  else if(event->button==3)
  {
    GdkPoint point = windowPointToPresentationPoint(eventToPoint(event));
    selection = Selection::Ptr(new Selection(point));
    for(auto listener : selectionListeners)
    {
      listener->onSelectionStart(point, shared_from_this<ViewInterface>());
    }
  }
}

void View::on_buttonRelease(GdkEventButton* event)
{
  if(event->button==1 && modifiermove==GDK_BUTTON1_MASK)
  {
    // End left-dragging
    modifiermove = 0;
    cachedPoint.x=0;
    cachedPoint.y=0;
  }
  else if(event->button==3 && selection)
  {
    selection->end = windowPointToPresentationPoint(eventToPoint(event));
    for(auto listener : selectionListeners)
    {
      listener->onSelectionEnd(selection, shared_from_this<ViewInterface>());
    }
    invalidate();
  }
}

void View::on_motion_notify(GdkEventMotion* event)
{
  if((event->state & GDK_BUTTON1_MASK) && modifiermove == GDK_BUTTON1_MASK)
  {
    int newx = x;
    int newy = y;

    if(zoom>=0)
    {
      const int pixelSize=1<<zoom;
      if(std::abs(event->x-cachedPoint.x)>=pixelSize*aspectRatio.x)
      {
        int delta = (event->x-cachedPoint.x)/(pixelSize*aspectRatio.x);
        cachedPoint.x += delta*pixelSize*aspectRatio.x;
        newx-=delta;
      }
      if(std::abs(event->y-cachedPoint.y)>=pixelSize*aspectRatio.y)
      {
        int delta = (event->y-cachedPoint.y)/(pixelSize*aspectRatio.y);
        cachedPoint.y += delta*pixelSize*aspectRatio.y;
        newy-=delta;
      }
    }
    else
    {
      const int pixelSize=1<<-zoom;
      newx-=(event->x-cachedPoint.x)*pixelSize*aspectRatio.x;
      newy-=(event->y-cachedPoint.y)*pixelSize*aspectRatio.y;
      cachedPoint = eventToPoint(event);
    }

    updateXY(newx, newy, OTHER);
  }
  else if((event->state & GDK_BUTTON3_MASK) && selection)
  {
    selection->end = windowPointToPresentationPoint(eventToPoint(event));
    for(auto listener : selectionListeners)
    {
      listener->onSelectionUpdate(selection, shared_from_this<ViewInterface>());
    }
    invalidate();
  }
}

void View::setFullScreen()
{
  gtk_window_fullscreen(window);
  gtk_widget_set_visible(toolbarArea, false);
  gtk_widget_set_visible(statusArea, false);
}

void View::unsetFullScreen()
{
  gtk_window_unfullscreen(window);
  gtk_widget_set_visible(toolbarArea, true);
  gtk_widget_set_visible(statusArea, true);
}

////////////////////////////////////////////////////////////////////////
// ViewInterface

void View::invalidate()
{
  gdk_window_invalidate_rect(gtk_widget_get_window(drawingArea), NULL, false);
}

ProgressInterface::Ptr View::getProgressInterface()
{
  return progressBarManager;
}

void View::addSideWidget(std::string title, GtkWidget* w)
{
  sidebarManager.addSideWidget(title, w);
}

void View::removeSideWidget(GtkWidget* w)
{
  sidebarManager.removeSideWidget(w);
}

void View::addToToolbar(GtkToolItem* ti)
{
  if(toolBarCount==0)
  {
    toolBarSeparator = gtk_separator_tool_item_new();
    g_object_set(G_OBJECT(toolBarSeparator),
                 "visible", true,
                 "draw", true,
                 NULL);
    gtk_toolbar_insert(toolBar, toolBarSeparator, -1);
  }

  g_object_set(G_OBJECT(ti), "visible", true, NULL);

  gtk_toolbar_insert(toolBar, ti, -1);
  toolBarCount++;
}

void View::removeFromToolbar(GtkToolItem* ti)
{
  gtk_container_remove(GTK_CONTAINER(toolBar), GTK_WIDGET(ti));
  toolBarCount--;

  if(toolBarCount==0)
  {
    gtk_container_remove(GTK_CONTAINER(toolBar), GTK_WIDGET(toolBarSeparator));
    toolBarSeparator=NULL;
  }
}

void View::registerSelectionListener(SelectionListener::Ptr listener)
{
  selectionListeners.push_back(listener);
}

void View::registerPostRenderer(PostRenderer::Ptr renderer)
{
  postRenderers.push_back(renderer);
}

void View::setStatusMessage(const std::string& message)
{
  gtk_statusbar_pop(statusBar, statusBarContextId);
  gtk_statusbar_push(statusBar, statusBarContextId, message.c_str());
}

PresentationInterface::Ptr View::getCurrentPresentation()
{
  return presentation;
}

static void tool_button_toggled(GtkToggleButton* button, gpointer data)
{
  static_cast<View*>(data)->toolButtonToggled(button);
}

void View::addToolButton(GtkToggleButton* button, ToolStateListener::Ptr callback)
{
  gdk_threads_enter();

  GtkToolItem* toolItem = gtk_tool_item_new();
  gtk_container_add(GTK_CONTAINER(toolItem), GTK_WIDGET(button));
  gtk_widget_set_visible(GTK_WIDGET(button), true);
  g_signal_connect(static_cast<gpointer>(button), "toggled", G_CALLBACK(tool_button_toggled), this);

  addToToolbar(toolItem);

  tools[button] = callback;
  if(tools.size() == 1)
  {
    gtk_toggle_button_set_active(button, true);
    gtk_widget_set_sensitive(GTK_WIDGET(button), false);
    callback->onEnable();
  }
  else
  {
	gtk_toggle_button_set_active(button, false);
	gtk_widget_set_sensitive(GTK_WIDGET(button), true);
  }

  gdk_threads_leave();
}

////////////////////////////////////////////////////////////////////////
// Helpers

GdkPoint View::windowPointToPresentationPoint(GdkPoint wp)
{
  GdkPoint result = {0,0};

  if(zoom>=0)
  {
    const int pixelSize=1<<zoom;
    result.x = x+(wp.x+(pixelSize*aspectRatio.x)/2)/(pixelSize*aspectRatio.x); // Round to make measurements snap
    result.y = y+(wp.y+(pixelSize*aspectRatio.y)/2)/(pixelSize*aspectRatio.y); // in the expected direction
  }
  else
  {
    const int pixelSize=1<<-zoom;
    result.x = x+wp.x*pixelSize*aspectRatio.x;
    result.y = y+wp.y*pixelSize*aspectRatio.y;
  }
  return result;
}

GdkPoint View::presentationPointToWindowPoint(GdkPoint pp)
{
  GdkPoint result = {0,0};

  if(zoom>=0)
  {
    const int pixelSize=1<<zoom;
    result.x = (pp.x-x)*pixelSize*aspectRatio.x;
    result.y = (pp.y-y)*pixelSize*aspectRatio.y;
  }
  else
  {
    const int pixelSize=1<<-zoom;
    result.x = (pp.x-x)/(pixelSize*aspectRatio.x);
    result.y = (pp.y-y)/(pixelSize*aspectRatio.y);
  }
  return result;
}

GdkPoint View::eventToPoint(GdkEventButton* event)
{
  GdkPoint result = {static_cast<gint>(event->x), static_cast<gint>(event->y)};
  return result;
}

GdkPoint View::eventToPoint(GdkEventMotion* event)
{
  GdkPoint result = {static_cast<gint>(event->x), static_cast<gint>(event->y)};
  return result;
}

void View::updateNewWindowMenu()
{
  GtkWidget* newWindow_menu_item = glade_xml_get_widget(scroomXml, "newWindow");

  GtkWidget* newWindow_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(newWindow_menu_item));
  if(!newWindow_menu)
    newWindow_menu=gtk_menu_new();
  g_object_ref_sink(G_OBJECT(newWindow_menu));

  std::map<PresentationInterface::WeakPtr,GtkWidget*>::iterator cur = presentations.begin();
  std::map<PresentationInterface::WeakPtr,GtkWidget*>::iterator end = presentations.end();

  while(cur!=end)
  {
    std::map<PresentationInterface::WeakPtr,GtkWidget*>::iterator next = cur;
    next++;

    //// Update menu
    PresentationInterface::Ptr p = cur->first.lock();
    GtkWidget* m = cur->second;
    if(p && m)
    {
      // Do nothing
    }
    else if(p && !m)
    {
      // Add a menu item
      std::string s = p->getTitle();
      if(!s.length())
        s = "Default";
      m=gtk_menu_item_new_with_label(s.c_str());
      gtk_widget_show(m);
      cur->second = m;
      gtk_container_add(GTK_CONTAINER(newWindow_menu), m);

      g_signal_connect (static_cast<gpointer>(m), "activate",
                        G_CALLBACK (on_newWindow_activate),
                        const_cast<PresentationInterface::WeakPtr*>(&cur->first));
    }
    else if(!p && m)
    {
      // Remove menu item, then remove this element from the map
      cur->second = NULL;
      gtk_widget_destroy(m);
      presentations.erase(cur);
    }
    else if(!p && !m)
    {
      // Remove this element from the map (menu already gone)
      presentations.erase(cur);
    }
    else
    {
      // This cannot happen
      printf("PANIC! Logic error in view.cc\n");
    }
    //// Done updating menu

    cur=next;
  }

  if(presentations.empty())
  {
    gtk_widget_set_sensitive(newWindow_menu_item, false);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(newWindow_menu_item), NULL);
  }
  else
  {
    gtk_widget_set_sensitive(newWindow_menu_item, true);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(newWindow_menu_item), newWindow_menu);
  }

  g_object_unref(G_OBJECT(newWindow_menu));
}

void View::updateXY(int x_, int y_, LocationChangeCause source)
{
  bool changed = false;
  if(this->x != x_)
  {
    this->x = x_;
    changed = true;
  }
  if(this->y != y_)
  {
    this->y = y_;
    changed = true;
  }

  if(changed)
  {
    if(source != SCROLLBAR)
      updateScrollbars();
    else
      updateRulers();

    if(source != TEXTBOX)
      updateTextbox();

    invalidate();
  }
}
