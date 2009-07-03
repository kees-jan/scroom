#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"


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
  gtk_widget_show (aboutdialog);
}


void
on_scroom_hide                         (GtkWidget       *widget,
                                        gpointer         user_data)
{
  gtk_main_quit();
}

