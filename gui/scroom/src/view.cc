#include "view.hh"

#include "pluginmanager.hh"
#include "callbacks.hh"

View::View(GladeXML* scroomXml, PresentationInterface* presentation)
  : scroomXml(scroomXml), presentation(presentation), drawingAreaWidth(0), drawingAreaHeight(0)
{
  PluginManager& pluginManager = PluginManager::getInstance();
  drawingArea = glade_xml_get_widget(scroomXml, "drawingarea");

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

  invalidate();
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
  invalidate();
}


////////////////////////////////////////////////////////////////////////
// Presentation events

void View::invalidate()
{
  gdk_window_invalidate_rect(gtk_widget_get_window(drawingArea), NULL, false);
}
