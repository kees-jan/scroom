#include "view.hh"

#include <cmath>

#include <sstream>

#include "pluginmanager.hh"
#include "callbacks.hh"

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

  
View::View(GladeXML* scroomXml, PresentationInterface* presentation)
  : scroomXml(scroomXml), presentation(NULL), drawingAreaWidth(0), drawingAreaHeight(0),
    zoom(0), x(0), y(0), vid(NULL), measurement(NULL), modifiermove(0)
{
  PluginManager& pluginManager = PluginManager::getInstance();
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

  cachedPoint.x=0;
  cachedPoint.y=0;
  
  on_newInterfaces_update(pluginManager.getNewInterfaces());
  on_configure();

  if(presentation)
  {
    setPresentation(presentation);
  }
}

View::~View()
{
  setPresentation(NULL);
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
      rect.width = drawingAreaWidth/pixelSize;
      rect.height = drawingAreaHeight/pixelSize;
    }
    else
    {
      // Zooming out. Smallest step is 1 window-pixel, which is more than one presentation-pixel
      int pixelSize = 1<<(-zoom);
      rect.width = drawingAreaWidth*pixelSize;
      rect.height = drawingAreaHeight*pixelSize;
    }
    
    presentation->redraw(vid, cr, rect, zoom);

    if(measurement)
    {
      GdkPoint start = presentationPointToWindowPoint(measurement->start);
      GdkPoint end = presentationPointToWindowPoint(measurement->end);
      printf("-->(%d, %d) - (%d,%d)\n", start.x,start.y, end.x, end.y);
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

void View::setPresentation(PresentationInterface* presentation)
{
  if(this->presentation)
  {
    this->presentation->close(vid);
    delete this->presentation;
    this->presentation=NULL;
  }

  this->presentation = presentation;

  if(this->presentation)
  {
    vid = presentation->open(this);
    presentationRect = presentation->getRect();
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
    unsigned int presentationHeight = presentationRect.height;
    unsigned int presentationWidth = presentationRect.width;
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
    bool found = false;
    for(bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(zoomItems), &iter);
        valid;
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(zoomItems), &iter))
    {
      GValue value={0};
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
  GValue value={0};
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
// Presentation events

void View::invalidate()
{
  gdk_window_invalidate_rect(gtk_widget_get_window(drawingArea), NULL, false);
}

GtkProgressBar* View::getProgressBar()
{
  return progressBar;
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
  GdkPoint result = {event->x, event->y};
  return result;
}

GdkPoint View::eventToPoint(GdkEventMotion* event)
{
  GdkPoint result = {event->x, event->y};
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
