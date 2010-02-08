#include <gtk/gtk.h>

#include <map>
#include <list>
#include <string>

#include <scroominterface.hh>

#include <presentationinterface.hh>

void on_scroom_hide(GtkWidget* widget, gpointer user_data);

void on_new_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_open_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_save_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_save_as_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_quit_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_cut_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_copy_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_paste_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_delete_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_about_activate(GtkMenuItem* menuitem, gpointer user_data);

gboolean on_drawingarea_expose_event(GtkWidget* widget, GdkEventExpose* event, gpointer user_data);

gboolean on_drawingarea_configure_event(GtkWidget* widget, GdkEventConfigure* event, gpointer user_data);

gboolean on_idle(gpointer user_data);

void on_done_loading_plugins();

void on_zoombox_changed(GtkComboBox* widget, gpointer user_data);

void on_scrollbar_value_changed(GtkAdjustment* adjustment, gpointer user_data);

void on_scroom_bootstrap (const std::list<std::string>& newFilenames);
 
void find_or_create_scroom(PresentationInterface::Ptr presentation);

void create_scroom(PresentationInterface::Ptr presentation);

void on_newInterfaces_update(const std::map<NewInterface*, std::string>& newInterfaces);
