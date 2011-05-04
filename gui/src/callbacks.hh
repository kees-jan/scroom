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

#include <gtk/gtk.h>

#include <map>
#include <list>
#include <string>

#include <scroom/scroominterface.hh>

#include <scroom/presentationinterface.hh>
#include "view.hh"

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

void on_presentation_created(PresentationInterface::Ptr p);

void on_view_created(View* v);

void on_view_destroyed(View* v);

void on_new_presentationobserver(PresentationObserver* po);

void on_new_viewobserver(ViewObserver* v);
