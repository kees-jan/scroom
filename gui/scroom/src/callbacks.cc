#include "callbacks.hh"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glade/glade.h>

#include <string>
#include <list>

#include <workinterface.hh>

#include "view.hh"
#include "pluginmanager.hh"

static std::string xmlFileName;
static GladeXML* aboutDialogXml=NULL;
static GtkWidget* aboutDialog=NULL;

static std::list<View*> views;

void on_scroom_hide (GtkWidget* widget, gpointer user_data)
{
  printf("hide\n");
  View* view = static_cast<View*>(user_data);
  views.remove(view);
  delete view;

  if(views.empty())
    gtk_main_quit();
}


void on_new_activate (GtkMenuItem* menuitem, gpointer user_data)
{
  create_scroom(NULL);
}


void on_open_activate (GtkMenuItem* menuitem, gpointer user_data)
{

}


void on_save_activate (GtkMenuItem* menuitem, gpointer user_data)
{

}


void on_save_as_activate (GtkMenuItem* menuitem, gpointer user_data)
{

}


void on_quit_activate (GtkMenuItem* menuitem, gpointer user_data)
{
  gtk_main_quit();
}


void on_cut_activate (GtkMenuItem* menuitem, gpointer user_data)
{

}


void on_copy_activate (GtkMenuItem* menuitem, gpointer user_data)
{

}


void on_paste_activate (GtkMenuItem* menuitem, gpointer user_data)
{

}


void on_delete_activate (GtkMenuItem* menuitem, gpointer user_data)
{

}


void on_about_activate (GtkMenuItem* menuitem, gpointer user_data)
{
  // GtkWidget* aboutdialog;
  // aboutdialog = create_aboutdialog ();
  gtk_dialog_run (GTK_DIALOG (aboutDialog ));
  gtk_widget_hide(aboutDialog);
  // gtk_widget_destroy (aboutdialog);
}


gboolean on_drawingarea_expose_event (GtkWidget* widget, GdkEventExpose* event, gpointer user_data)
{
  printf("expose\n");

  ////////////////////////////////////////////////////////////////////////

  GdkWindow* win = gtk_widget_get_window(widget);
  GdkGC* gc = gdk_gc_new(GDK_DRAWABLE(win));
  // gdk_gc_set_foreground(gc, ??);
  // gdk_gc_set_background(gc, ??);
  gdk_draw_line(GDK_DRAWABLE(win), gc, 50, 50, 50, 100);
  gdk_draw_line(GDK_DRAWABLE(win), gc, 50, 100, 100, 100);
  gdk_draw_line(GDK_DRAWABLE(win), gc, 100, 50, 100, 100);
  gdk_draw_line(GDK_DRAWABLE(win), gc, 50, 50, 100, 50);
  ////////////////////////////////////////////////////////////////////////
 

  ////////////////////////////////////////////////////////////////////////
  cairo_t* cr;
  char buffer[] = "Hello world!";

  cr = gdk_cairo_create(widget->window);

  cairo_move_to(cr, 30, 30);
  cairo_show_text(cr, buffer);

  cairo_destroy(cr);
  ////////////////////////////////////////////////////////////////////////
  return FALSE;
}


gboolean on_drawingarea_configure_event (GtkWidget* widget, GdkEventConfigure* event, gpointer user_data)
{
  printf("configure\n");
  return FALSE;
}


gboolean on_idle (gpointer user_data)
{
  if(user_data==NULL)
  {
    return 0;
  }
  else
  {
    return reinterpret_cast<WorkInterface*>(user_data)->doWork();
  }
}

void on_scroom_bootstrap ()
{
  printf("Bootstrapping Scroom...\n");
  startPluginManager();

  aboutDialogXml = glade_xml_new("scroom.glade", "aboutDialog", NULL);
  if(aboutDialogXml!=NULL)
  {
    xmlFileName = "scroom.glade";
    aboutDialog = glade_xml_get_widget(aboutDialogXml, "aboutDialog");
  }
  else
  {
    printf("Opening xml failed\n");
    exit(-1);
  }
}
 
void create_scroom(PresentationInterface* presentation)
{
  GladeXML* xml = glade_xml_new(xmlFileName.c_str(), "scroom", NULL);

  if(xml==NULL)
  {
    printf("Opening xml failed\n");
    exit(-1);
  }

  View* view = new View(xml, presentation);
  views.push_back(view);

  GtkWidget* scroom = glade_xml_get_widget(xml, "scroom");
  GtkWidget* newMenuItem = glade_xml_get_widget(xml, "new");
  GtkWidget* quitMenuItem = glade_xml_get_widget(xml, "quit");
  GtkWidget* aboutMenuItem = glade_xml_get_widget(xml, "about");
  GtkWidget* drawingArea = glade_xml_get_widget(xml, "drawingarea");

  g_signal_connect ((gpointer) scroom, "hide", G_CALLBACK (on_scroom_hide), view);
  // g_signal_connect ((gpointer) open, "activate",
  //                   G_CALLBACK (on_open_activate),
  //                   view);
  // g_signal_connect ((gpointer) save, "activate",
  //                   G_CALLBACK (on_save_activate),
  //                   view);
  // g_signal_connect ((gpointer) save_as, "activate",
  //                   G_CALLBACK (on_save_as_activate),
  //                   view);
  g_signal_connect ((gpointer) newMenuItem, "activate", G_CALLBACK (on_new_activate), view);
  g_signal_connect ((gpointer) quitMenuItem, "activate", G_CALLBACK (on_quit_activate), view);
  // g_signal_connect ((gpointer) cut, "activate",
  //                   G_CALLBACK (on_cut_activate),
  //                   view);
  // g_signal_connect ((gpointer) copy, "activate",
  //                   G_CALLBACK (on_copy_activate),
  //                   view);
  // g_signal_connect ((gpointer) paste, "activate",
  //                   G_CALLBACK (on_paste_activate),
  //                   view);
  // g_signal_connect ((gpointer) delete, "activate",
  //                   G_CALLBACK (on_delete_activate),
  //                   view);
  g_signal_connect ((gpointer) aboutMenuItem, "activate", G_CALLBACK (on_about_activate), view);
  g_signal_connect ((gpointer) drawingArea, "expose_event", G_CALLBACK (on_drawingarea_expose_event), view);
  g_signal_connect ((gpointer) drawingArea, "configure_event", G_CALLBACK (on_drawingarea_configure_event), view);
}
