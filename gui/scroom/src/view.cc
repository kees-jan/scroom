#include "view.hh"

#include "pluginmanager.hh"
#include "callbacks.hh"

static const char *zoomfactor[] =
  {
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
static const int absMaxZoom=3;
static const int absMinZoom=-30;

enum
  {
    COLUMN_TEXT,
    COLUMN_ZOOM,
    N_COLUMNS
  };

  
View::View(GladeXML* scroomXml, PresentationInterface* presentation)
  : scroomXml(scroomXml), presentation(presentation), drawingAreaWidth(0), drawingAreaHeight(0),
    zoom(0), minZoom(absMaxZoom)
{
  PluginManager& pluginManager = PluginManager::getInstance();
  drawingArea = glade_xml_get_widget(scroomXml, "drawingarea");
  vscrollbar = GTK_VSCROLLBAR(glade_xml_get_widget(scroomXml, "vscrollbar"));
  hscrollbar = GTK_HSCROLLBAR(glade_xml_get_widget(scroomXml, "hscrollbar"));
  vscrollbaradjustment = gtk_range_get_adjustment(GTK_RANGE(vscrollbar));
  hscrollbaradjustment = gtk_range_get_adjustment(GTK_RANGE(hscrollbar));
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
  
  if(presentation)
  {
    presentationRect = presentation->getRect();
  }
  
  on_newInterfaces_update(pluginManager.getNewInterfaces());
  on_configure();

}

void View::redraw(cairo_t* cr)
{
  if(presentation)
  {
    GdkRectangle rect;
    presentation->redraw(cr, rect, 0);
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
    delete this->presentation;
    this->presentation=NULL;
  }

  this->presentation = presentation;
  presentationRect = presentation->getRect();
  
  updateZoom();
  updateScrollbars();
  invalidate();
}

void View::updateScrollbars()
{
  if(!presentation)
  {
    presentationRect.x=0;
    presentationRect.y=0;
    presentationRect.width=0;
    presentationRect.height=0;
  }

  gtk_adjustment_configure(vscrollbaradjustment, 100, 0, 500, 1, 100, 100);
  gtk_adjustment_configure(hscrollbaradjustment, 100, 0, 500, 1, 100, 100);
}

void View::updateZoom()
{
  int newMinZoom = -15;
  printf("updateZoom\n");

  int zMax = absMaxZoom - newMinZoom;
  zMax = std::max(zMax, absMaxZoom-zoom);
  zMax = std::min((unsigned int)zMax, sizeof(zoomfactor)/sizeof(zoomfactor[0]));
  bool zoomFound = false;
  
  gtk_list_store_clear(zoomItems);
  for(int z=0; z<zMax; z++)
  {
    GtkTreeIter iter;
    gtk_list_store_insert_with_values(zoomItems, &iter, z,
                                      COLUMN_TEXT, zoomfactor[z],
                                      COLUMN_ZOOM, absMaxZoom-z,
                                      -1);

    if(zoom == absMaxZoom-z)
    {
      gtk_combo_box_set_active_iter(zoomBox, &iter);
      zoomFound = true;
    }
  }
  
  minZoom = newMinZoom;
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
  printf("New drawing area size: %d, %d\n", newWidth, newHeight);
  drawingAreaHeight = newHeight;
  drawingAreaWidth = newWidth;
  updateZoom();
  updateScrollbars();
  invalidate();
}


////////////////////////////////////////////////////////////////////////
// Presentation events

void View::invalidate()
{
  gdk_window_invalidate_rect(gtk_widget_get_window(drawingArea), NULL, false);
}
