/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/presentationinterface.hh>
#include <scroom/viewinterface.hh>

void PresentationBase::open(ViewInterface::WeakPtr vi)
{
  viewAdded(vi);

  std::list<Viewable::Ptr> const observers = getObservers();

  for(Viewable::Ptr const& observer: observers)
  {
    observer->open(vi);
  }
}

void PresentationBase::close(ViewInterface::WeakPtr vi)
{
  std::list<Viewable::Ptr> const observers = getObservers();

  for(Viewable::Ptr const& observer: observers)
  {
    observer->close(vi);
  }

  viewRemoved(vi);
}

void PresentationBase::observerAdded(Viewable::Ptr const& viewable, Scroom::Bookkeeping::Token const&)
{
  std::set<ViewInterface::WeakPtr> views = getViews();

  for(ViewInterface::WeakPtr const& view: views)
  {
    viewable->open(view);
  }
}

/**
 * Base method that shows image metadata window for unimplemented
 * presentations on which the user clicks the metadata button.
 *
 */
void PresentationBase::showMetadata()
{
  GtkWidget*  dialog;
  GtkWidget*  label;
  GtkBuilder* builder;
  GtkWidget*  box;

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "popup.builder", NULL);
  printf("Creating the properties window.\n");

  // create top level window and set its size
  dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(dialog), "Properties");
  gtk_window_set_decorated(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);
  gtk_builder_connect_signals(builder, dialog);
  g_object_unref(G_OBJECT(builder));

  // create label in window and paste it in container
  label = gtk_label_new("NOT IMPLEMENTED FOR THIS PRESENTATION");
  box   = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(box), label, true, false, 5);
  gtk_container_add(GTK_CONTAINER(dialog), box);

  // show widgets in window dialog
  gtk_widget_show_all(dialog);
  gtk_widget_grab_focus(dialog);
}
