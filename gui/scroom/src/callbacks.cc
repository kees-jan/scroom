#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cairo.h>
#include <gtk/gtk.h>

extern "C"
{
#include "callbacks.h"
#include "interface.h"
#include "support.h"
}

#include "view.hh"



void
on_scroom_hide                         (GtkWidget       *widget,
                                        gpointer         user_data)
{
  gtk_main_quit();
}


gboolean
on_scroom_destroy_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_main_quit();
  return FALSE;
}


void
on_new_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_open_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_save_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_save_as_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_main_quit();
}


void
on_cut_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_copy_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_paste_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_delete_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *aboutdialog;
  aboutdialog = create_aboutdialog ();
  gtk_dialog_run (GTK_DIALOG (aboutdialog ));
  gtk_widget_destroy (aboutdialog);
}


gboolean
on_drawingarea_expose_event            (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
  printf("expose\n");
  cairo_t *cr;
  char* buffer = "Hello world!";

  cr = gdk_cairo_create(widget->window);

  cairo_move_to(cr, 30, 30);
  cairo_show_text(cr, buffer);

  cairo_destroy(cr);

  return FALSE;
}


gboolean
on_drawingarea_configure_event         (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{

  return FALSE;
}

