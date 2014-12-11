/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

#include "roiui.hh"

#include <stdio.h>
#include <gtk/gtk.h>

enum
{
  COL_NAME = 0,
  COL_POINTER,
  NUM_COLS
} ;


static GtkTreeModel *
create_and_fill_model (void)
{
  GtkTreeStore  *store;
  GtkTreeIter    iter;
  GtkTreeIter    iter2;

  store = gtk_tree_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_POINTER);

  /* Append a row and fill in some data */
  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      COL_NAME, "Heinz El-Mann",
                      COL_POINTER, 51,
                      -1);

  /* append another row and fill in some data */
  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      COL_NAME, "Jane Doe",
                      COL_POINTER, 23,
                      -1);

  gtk_tree_store_append (store, &iter2, &iter);
  gtk_tree_store_set (store, &iter2,
                      COL_NAME, "Kees-Jan Dijkzeul",
                      COL_POINTER, 42,
                      -1);

  /* ... and a third row */
  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      COL_NAME, "Joe Bungop",
                      COL_POINTER, 91,
                      -1);

  return GTK_TREE_MODEL (store);
}

static GtkWidget *
create_view_and_model (void)
{
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view;

  view = gtk_tree_view_new ();

  /* --- Column #1 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,
                                               "Name",
                                               renderer,
                                               "text", COL_NAME,
                                               NULL);

  model = create_and_fill_model ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  /* The tree view has acquired its own reference to the
   *  model, so we can drop ours. That way the model will
   *  be freed automatically when the tree view is destroyed */

  g_object_unref (model);

  return view;
}



RoiUi::Ptr RoiUi::create(const std::string& fileName, ScroomInterface::Ptr const& scroomInterface)
{
  Ptr result = Ptr(new RoiUi());
  result->init(fileName, scroomInterface);

  return result;
}

RoiUi::RoiUi()
{}

void RoiUi::init(std::string const& fileName, ScroomInterface::Ptr const& scroomInterface)
{
  list = Scroom::Roi::parse(fileName);
  std::set<ViewObservable::Ptr> viewObservables = list->instantiate(scroomInterface, fileName);

  if(!viewObservables.empty() && !list->regions.empty())
  {
    for(auto const & viewObservable: viewObservables)
    {
      stuff.push_back(viewObservable->registerStrongObserver(shared_from_this<RoiUi>()));
    }
  }
}

void RoiUi::open(ViewInterface::WeakPtr viWeak)
{
  ViewInterface::Ptr vi = viWeak.lock();
  if(vi)
  {
    GtkWidget* view = create_view_and_model();

    vi->addSideWidget("Roi", view);
  }
}

void RoiUi::close(ViewInterface::WeakPtr)
{}
