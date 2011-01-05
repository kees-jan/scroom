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

#include "colormapprovider.hh"

#include "colormaps.hh"

////////////////////////////////////////////////////////////////////////

void on_colormap_selected (GtkTreeView *tv, gpointer user_data)
{
  ColormapProvider* cmp = (ColormapProvider*)user_data;
  cmp->on_colormap_selected(tv);
}

////////////////////////////////////////////////////////////////////////

/** Names of the columns in the GtkListStore */
enum
  {
    COLUMN_NAME,
    COLUMN_POINTER,
    N_COLUMNS
  };

////////////////////////////////////////////////////////////////////////

ColormapProvider::Ptr ColormapProvider::create(PresentationInterface::Ptr p)
{
  Colormappable::Ptr c = boost::dynamic_pointer_cast<Colormappable, PresentationInterface>(p);
  ColormapProvider::Ptr result;
  if(c)
  {
    result = ColormapProvider::Ptr(new ColormapProvider(c));

    Scroom::Utils::Registration r = c->registerStrongObserver(result);
    result->registration = r;
  }
  else
    printf("PANIC: Presentation doesn't implement Colormappable\n");

  return result;
}

ColormapProvider::ColormapProvider(Colormappable::Ptr c)
  : colormappable(c), colormaps(NULL)
{
  unsigned int numColors = c->getNumberOfColors();
  std::list<Colormap::ConstPtr> maps = Colormaps::getInstance().getColormaps();

  colormaps = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);

  {
    // Add the original colormap, if any
    Colormap::Ptr orig = c->getOriginalColormap();
    if(orig)
    {
      Colormap::ConstPtr* cc = new Colormap::ConstPtr(orig);
      GtkTreeIter iter;
      gtk_list_store_append(colormaps, &iter);
      gtk_list_store_set(colormaps, &iter,
                         COLUMN_NAME, (*cc)->name.c_str(),
                         COLUMN_POINTER, cc,
                         -1);
    }
  }
    
  while(!maps.empty())
  {
    if(maps.front()->colors.size()>=numColors)
    {
      Colormap::ConstPtr* cc = new Colormap::ConstPtr(maps.front());

      GtkTreeIter iter;
      gtk_list_store_append(colormaps, &iter);
      gtk_list_store_set(colormaps, &iter,
                         COLUMN_NAME, (*cc)->name.c_str(),
                         COLUMN_POINTER, cc,
                         -1);
    }

    maps.pop_front();
  }
}

ColormapProvider::~ColormapProvider()
{
  // Clear out all the smart pointers
  printf("ColormapProvider: Destructing...\n");
  if(colormaps)
  {
    GtkTreeIter iter;
    while(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(colormaps), &iter))
    {
      gpointer* pointer = NULL;
      gtk_tree_model_get(GTK_TREE_MODEL(colormaps), &iter, COLUMN_POINTER, &pointer, -1);
      Colormap::Ptr* colormap = (Colormap::Ptr*)pointer;
      delete colormap;
      gtk_list_store_remove(colormaps, &iter);
    }
    g_object_unref(colormaps);
  }
  colormaps = NULL;
}

void ColormapProvider::open(ViewInterface* vi)
{
  printf("ColormapProvider: Adding a view.\n");
  GtkTreeView* tv = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(colormaps)));
  GtkCellRenderer* txt = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
  gtk_tree_view_insert_column_with_attributes(tv, -1, "Name", txt, "text", COLUMN_NAME, NULL);
  g_signal_connect ((gpointer)tv, "cursor_changed", G_CALLBACK (::on_colormap_selected), this);
  views[vi]=tv;

  vi->addSideWidget("Colormap", GTK_WIDGET(tv));
}

void ColormapProvider::close(ViewInterface* vi)
{
  printf("ColormapProvider: Removing a view.\n");
  std::map<ViewInterface*, GtkTreeView*>::iterator cur = views.find(vi);
  if(cur != views.end())
    views.erase(cur);
  if(views.empty())
  {
    printf("ColormapProvider: Last view has gone.\n");
  }
}

void ColormapProvider::on_colormap_selected(GtkTreeView* tv)
{
  Colormappable::Ptr c = colormappable.lock();
  if(c)
  {
    GtkTreeSelection* ts = gtk_tree_view_get_selection(tv);
    GtkTreeIter iter;
    GtkTreeModel* model = NULL;
    bool selected = gtk_tree_selection_get_selected(ts, &model, &iter);
    if(selected)
    {
      if(gtk_list_store_iter_is_valid(colormaps, &iter))
      {
        gpointer* pointer = NULL;
        gtk_tree_model_get(GTK_TREE_MODEL(colormaps), &iter, COLUMN_POINTER, &pointer, -1);
        Colormap::Ptr& colormap = *(Colormap::Ptr*)pointer;
        c->setColormap(colormap);
      }
    }
  }
  else
    printf("PANIC: Colormappable Presentation is gone??\n");
}
