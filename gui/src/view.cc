/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "view.hh"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cmath>

#include <sstream>

#include <glib-object.h>

#include "pluginmanager.hh"
#include "callbacks.hh"

#define G_VALUE_INIT {0,{{0}}}

static const char *zoomfactor[] =
  {
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
static const int MaxZoom=4;

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

#if MUTRACX_HACKS

void gtk_adjustment_configure(GtkAdjustment* adj,
                              gdouble value, gdouble lower, gdouble upper,
                              gdouble step_increment,
                              gdouble page_increment, gdouble page_size)
{
    g_object_set(G_OBJECT(adj),
                 "value",          value,
                 "lower",          lower,
                 "upper",          upper,
                 "step-increment", step_increment,
                 "page-increment", page_increment,
                 "page-size",      page_size,
                 NULL);
    gtk_adjustment_changed(adj);
}

GdkWindow* gtk_widget_get_window(GtkWidget *widget)
{
  return widget->window;
}

#endif

////////////////////////////////////////////////////////////////////////
  
View::View(GladeXML* scroomXml, PresentationInterface::Ptr presentation)
  : scroomXml(scroomXml), presentation(), sidebarManager(),
    drawingAreaWidth(0), drawingAreaHeight(0),
    zoom(0), x(0), y(0), measurement(NULL), modifiermove(0)
{
  PluginManager& pluginManager = PluginManager::getInstance();
  window = GTK_WINDOW(glade_xml_get_widget(scroomXml, "scroom"));
  drawingArea = glade_xml_get_widget(scroomXml, "drawingarea");
  vscrollbar = GTK_VSCROLLBAR(glade_xml_get_widget(scroomXml, "vscrollbar"));
  hscrollbar = GTK_HSCROLLBAR(glade_xml_get_widget(scroomXml, "hscrollbar"));
  vscrollbaradjustment = gtk_range_get_adjustment(GTK_RANGE(vscrollbar));
  hscrollbaradjustment = gtk_range_get_adjustment(GTK_RANGE(hscrollbar));
  vruler = GTK_RULER(glade_xml_get_widget(scroomXml, "vruler"));
  hruler = GTK_RULER(glade_xml_get_widget(scroomXml, "hruler"));

  zoomBox = GTK_COMBO_BOX(glade_xml_get_widget(scroomXml, "zoomboxcombo"));
  zoomItems = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_INT);

  gtk_combo_box_set_model(zoomBox, GTK_TREE_MODEL(zoomItems));
  GtkCellRenderer* txt = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
  gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(zoomBox),
                           txt,
                           true);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(zoomBox), txt,
                                 "text", COLUMN_TEXT,
                                 NULL);

  progressBar = GTK_PROGRESS_BAR(glade_xml_get_widget(scroomXml, "progressbar"));
  statusBar = GTK_STATUSBAR(glade_xml_get_widget(scroomXml, "statusbar"));
  statusBarContextId = gtk_statusbar_get_context_id(statusBar, "View");

  GtkWidget* panelWindow = glade_xml_get_widget(scroomXml, "panelWindow");
  GtkBox* panel = GTK_BOX(glade_xml_get_widget(scroomXml, "panel"));
  sidebarManager.setWidgets(panelWindow, panel);
  toolBar = GTK_TOOLBAR(glade_xml_get_widget(scroomXml, "toolbar"));
  toolBarSeparator = NULL;
  toolBarCount = 0;

  cachedPoint.x=0;
  cachedPoint.y=0;
  
  on_newInterfaces_update(pluginManager.getNewInterfaces());
  updateNewWindowMenu();  
  on_configure();

  if(presentation)
  {
    setPresentation(presentation);
  }
}

View::~View()
{
  setPresentation(PresentationInterface::Ptr());
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
    }
    
    presentation->redraw(this, cr, rect, zoom);

    if(measurement)
    {
      GdkPoint start = presentationPointToWindowPoint(measurement->start);
      GdkPoint end = presentationPointToWindowPoint(measurement->end);
      cairo_set_line_width(cr, 1);
      cairo_set_source_rgb(cr, 0.75, 0, 0); // Dark Red
      drawCross(cr, start);
      drawCross(cr, end);
      cairo_stroke(cr);
      cairo_set_source_rgb(cr, 1, 0, 0); // Red
      cairo_move_to(cr, start.x, start.y);
      cairo_line_to(cr, end.x, end.y);
      cairo_stroke(cr);
    }
  }
  else
  {
    char buffer[] = "View says \"Hi\"";

    cairo_move_to(cr, 50, 50);
    cairo_show_text(cr, buffer);
  }
}

bool View::hasPresentation()
{
  return presentation!=NULL;
}

void View::setPresentation(PresentationInterface::Ptr presentation)
{
  if(this->presentation)
  {
    this->presentation->close(this);
    this->presentation.reset();
  }

  this->presentation = presentation;

  if(this->presentation)
  {
    presentation->open(this);
    presentationRect = presentation->getRect();
    std::string s = presentation->getTitle();
    if(s.length())
      s = "Scroom - " + s;
    else
      s = "Scroom";
    gtk_window_set_title(window, s.c_str());
  }
  updateZoom();
  updateScrollbars();
  invalidate();
}

void View::updateScrollbar(GtkAdjustment* adj, int zoom, int value, int presentationStart, int presentationSize, int windowSize)
{
  if(zoom>=0)
  {
    // Zooming in. Smallest step is 1 presentation pixel, which is more than one window-pixel
    int pixelSize = 1<<zoom;
    presentationStart -= windowSize/pixelSize/2;
    presentationSize += windowSize/pixelSize;
    
    gtk_adjustment_configure(adj, value, presentationStart, presentationStart+presentationSize,
                             1, 3*windowSize/pixelSize/4, windowSize/pixelSize);
  }
  else
  {
    // Zooming out. Smallest step is 1 window-pixel, which is more than one presentation-pixel
    int pixelSize = 1<<(-zoom);
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
                    presentationRect.x, presentationRect.width, drawingAreaWidth);
    updateScrollbar(vscrollbaradjustment, zoom, y,
                    presentationRect.y, presentationRect.height, drawingAreaHeight);
    updateRulers();
  }
  else
  {
    gtk_widget_set_sensitive(GTK_WIDGET(vscrollbar), false);
    gtk_widget_set_sensitive(GTK_WIDGET(hscrollbar), false);
  }

}

void View::updateZoom()
{
  if(presentation)
  {
    int presentationHeight = presentationRect.height;
    int presentationWidth = presentationRect.width;
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
    bool zoomFound = false;
  
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
        zoomFound = true;
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
    gtk_ruler_set_range(hruler, x, x + 1.0*drawingAreaWidth/pixelSize, 0, presentationRect.x + presentationRect.width);
    gtk_ruler_set_range(vruler, y, y + 1.0*drawingAreaHeight/pixelSize, 0, presentationRect.y + presentationRect.height);
  }
  else
  {
    // Zooming out. Smallest step is 1 window-pixel, which is more than one presentation-pixel
    int pixelSize = 1<<(-zoom);
    gtk_ruler_set_range(hruler, x, x + drawingAreaWidth*pixelSize, 0, presentationRect.x + presentationRect.width);
    gtk_ruler_set_range(vruler, y, y + drawingAreaHeight*pixelSize, 0, presentationRect.y + presentationRect.height);
  }
}

////////////////////////////////////////////////////////////////////////
// Scroom events
  
void View::on_newInterfaces_update(const std::map<NewInterface*, std::string>& newInterfaces)
{
  GtkWidget* new_menu_item = glade_xml_get_widget(scroomXml, "new");

  if(newInterfaces.empty())
  {
    gtk_widget_set_sensitive(new_menu_item, false);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (new_menu_item), NULL);
  }
  else
  {
    gtk_widget_set_sensitive(new_menu_item, true);

    GtkWidget* new_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (new_menu_item), new_menu);

    for(std::map<NewInterface*, std::string>::const_iterator cur=newInterfaces.begin();
        cur != newInterfaces.end();
        cur++)
    {
      GtkWidget* menu_item = gtk_menu_item_new_with_label(cur->second.c_str());
      gtk_widget_show (menu_item);
      gtk_container_add (GTK_CONTAINER (new_menu), menu_item);

      g_signal_connect ((gpointer) menu_item, "activate", G_CALLBACK (on_new_activate), cur->first);
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
    x+=(drawingAreaWidth-newWidth)/pixelSize/2;
    y+=(drawingAreaHeight-newHeight)/pixelSize/2;
  }
  else
  {
    const int pixelSize = 1<<-zoom;
    x+=(drawingAreaWidth-newWidth)*pixelSize/2;
    y+=(drawingAreaHeight-newHeight)*pixelSize/2;
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

void View::on_zoombox_changed(int newZoom, int mousex, int mousey)
{
  if(newZoom!=zoom)
  {
    if(zoom>=0)
    {
      const int pixelSize = 1<<zoom;
      x+=mousex/pixelSize;
      y+=mousey/pixelSize;
    }
    else
    {
      const int pixelSize = 1<<-zoom;
      x+=mousex*pixelSize;
      y+=mousey*pixelSize;
    }

    if(newZoom>=0)
    {
      const int pixelSize = 1<<newZoom;
      x-=mousex/pixelSize;
      y-=mousey/pixelSize;
    }
    else
    {
      const int pixelSize = 1<<-newZoom;
      x-=mousex*pixelSize;
      y-=mousey*pixelSize;
    }
    
    zoom = newZoom;
    updateScrollbars();
    invalidate();
  }
}

void View::on_scrollbar_value_changed(GtkAdjustment* adjustment)
{
  if(adjustment == vscrollbaradjustment)
  {
    y = (int)gtk_adjustment_get_value(adjustment);
  }
  else
  {
    x = (int)gtk_adjustment_get_value(adjustment);
  }
  updateRulers();
  invalidate();
}

void View::on_buttonPress(GdkEventButton* event)
{
  if(event->button==1 && modifiermove==0)
  {
    // Begin left-dragging
    modifiermove = GDK_BUTTON1_MASK;
    cachedPoint = eventToPoint(event);
  }
  else if(event->button==3 && modifiermove==0)
  {
    // Begin measuring distance
    modifiermove = GDK_BUTTON3_MASK;
    if(measurement)
    {
      delete measurement;
    }
    cachedPoint = windowPointToPresentationPoint(eventToPoint(event));
    measurement = new Measurement(cachedPoint);
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
  else if(event->button==3 && modifiermove==GDK_BUTTON3_MASK)
  {
    // End measuring distance
    modifiermove = 0;
    if(measurement)
    {
      measurement->end = windowPointToPresentationPoint(eventToPoint(event));
    }
    cachedPoint.x=0;
    cachedPoint.y=0;
  }
}

void View::on_motion_notify(GdkEventMotion* event)
{
  if((event->state & GDK_BUTTON1_MASK) && modifiermove == GDK_BUTTON1_MASK)
  {
    bool moved=false;
    
    if(zoom>=0)
    {
      const int pixelSize=1<<zoom;
      if(std::abs(event->x-cachedPoint.x)>=pixelSize)
      {
        int delta = (int(event->x)-cachedPoint.x)/pixelSize;
        cachedPoint.x += delta*pixelSize;
        x-=delta;
        moved=true;
      }
      if(std::abs(event->y-cachedPoint.y)>=pixelSize)
      {
        int delta = (int(event->y)-cachedPoint.y)/pixelSize;
        cachedPoint.y += delta*pixelSize;
        y-=delta;
        moved=true;
      }
    }
    else
    {
      const int pixelSize=1<<-zoom;
      x-=(event->x-cachedPoint.x)*pixelSize;
      y-=(event->y-cachedPoint.y)*pixelSize;
      cachedPoint = eventToPoint(event);
      moved=true;
    }

    if(moved)
    {
      updateScrollbars();
      updateRulers();
      invalidate();
    }
  }
  else if((event->state & GDK_BUTTON3_MASK) && modifiermove == GDK_BUTTON3_MASK)
  {
    bool moved=false;

    cachedPoint = windowPointToPresentationPoint(eventToPoint(event));
    if(measurement && !measurement->endsAt(cachedPoint))
    {
      measurement->end = cachedPoint;
      moved = true;
    }
    
    if(moved)
    {
      invalidate();
      displayMeasurement();
    }
  }
}


////////////////////////////////////////////////////////////////////////
// ViewInterface

void View::invalidate()
{
  gdk_window_invalidate_rect(gtk_widget_get_window(drawingArea), NULL, false);
}

GtkProgressBar* View::getProgressBar()
{
  return progressBar;
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
    gtk_toolbar_insert(toolBar, toolBarSeparator, -1);
  }

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

////////////////////////////////////////////////////////////////////////
// Helpers

GdkPoint View::windowPointToPresentationPoint(GdkPoint wp)
{
  GdkPoint result = {0,0};

  if(zoom>=0)
  {
    const int pixelSize=1<<zoom;
    result.x = x+(wp.x+pixelSize/2)/pixelSize; // Round to make measurements snap
    result.y = y+(wp.y+pixelSize/2)/pixelSize; // in the expected direction
  }
  else
  {
    const int pixelSize=1<<-zoom;
    result.x = x+wp.x*pixelSize;
    result.y = y+wp.y*pixelSize;
  }
  return result;
}

GdkPoint View::presentationPointToWindowPoint(GdkPoint pp)
{
  GdkPoint result = {0,0};

  if(zoom>=0)
  {
    const int pixelSize=1<<zoom;
    result.x = (pp.x-x)*pixelSize;
    result.y = (pp.y-y)*pixelSize;
  }
  else
  {
    const int pixelSize=1<<-zoom;
    result.x = (pp.x-x)/pixelSize;
    result.y = (pp.y-y)/pixelSize;
  }
  return result;
}
  
GdkPoint View::eventToPoint(GdkEventButton* event)
{
  GdkPoint result = {(gint)event->x, (gint)event->y};
  return result;
}

GdkPoint View::eventToPoint(GdkEventMotion* event)
{
  GdkPoint result = {(gint)event->x, (gint)event->y};
  return result;
}

void View::drawCross(cairo_t* cr, GdkPoint p)
{
  static const int size = 10;
  cairo_move_to(cr, p.x-size, p.y);
  cairo_line_to(cr, p.x+size, p.y);
  cairo_move_to(cr, p.x, p.y-size);
  cairo_line_to(cr, p.x, p.y+size);
}

void View::setStatusMessage(const std::string& message)
{
  gtk_statusbar_pop(statusBar, statusBarContextId);
  gtk_statusbar_push(statusBar, statusBarContextId, message.c_str());
}

void View::displayMeasurement()
{
  std::ostringstream s;
  s.precision(1);
  fixed(s);

  if(measurement)
  {
    s << "l: " << measurement->length()
      << ", dx: " << measurement->width()
      << ", dy: " << measurement->height()
      << ", from: ("<< measurement->start.x << "," << measurement->start.y << ")"
      << ", to: ("<< measurement->end.x << "," << measurement->end.y << ")";
  }

  setStatusMessage(s.str());
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

      g_signal_connect ((gpointer)m, "activate",
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
