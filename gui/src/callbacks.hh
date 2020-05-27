/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gtk/gtk.h>

#include <map>
#include <list>
#include <string>

#include <scroom/scroominterface.hh>

#include <scroom/presentationinterface.hh>
#include "view.hh"

typedef std::map<std::string, std::list<std::string> > FileNameMap;
extern const std::string REGULAR_FILES;

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

void on_fullscreen_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_close_activate(GtkMenuItem* menuitem, gpointer user_data);

void on_about_activate(GtkMenuItem* menuitem, gpointer user_data);

gboolean on_drawingarea_expose_event(GtkWidget* widget, GdkEventExpose* event, gpointer user_data);

gboolean on_drawingarea_configure_event(GtkWidget* widget, GdkEventConfigure* event, gpointer user_data);

gboolean on_idle(gpointer user_data);

void on_done_loading_plugins();

void on_zoombox_changed(GtkComboBox* widget, gpointer user_data);

void on_scrollbar_value_changed(GtkAdjustment* adjustment, gpointer user_data);

void on_textbox_value_changed(GtkEditable* editable, gpointer user_data);

void on_scroom_bootstrap (const FileNameMap& newFilenames);

void find_or_create_scroom(PresentationInterface::Ptr presentation);

void create_scroom(PresentationInterface::Ptr presentation);

void on_newPresentationInterfaces_update(const std::map<NewPresentationInterface::Ptr, std::string>& newPresentationInterfaces);

void on_presentation_created(PresentationInterface::Ptr p);

void on_view_created(View::Ptr v);

void on_view_destroyed(View* v);

void on_new_presentationobserver(PresentationObserver::Ptr po);

void on_new_viewobserver(ViewObserver::Ptr v);

void on_presentation_possibly_destroyed();

PresentationInterface::Ptr on_load_presentation(const std::string& filename);
