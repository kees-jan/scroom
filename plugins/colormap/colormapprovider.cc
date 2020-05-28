/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "colormapprovider.hh"

#include "colormaps.hh"

////////////////////////////////////////////////////////////////////////

namespace
{
  void on_colormap_selected(GtkTreeView *tv, gpointer user_data)
  {
    Scroom::ColormapImpl::ColormapProvider* cmp = static_cast<Scroom::ColormapImpl::ColormapProvider*>(user_data);
    cmp->on_colormap_selected(tv);
  }
}
////////////////////////////////////////////////////////////////////////

namespace Scroom
{
  namespace ColormapImpl
  {
    /** Names of the columns in the GtkListStore */
    enum
    {
      COLUMN_NAME, COLUMN_POINTER, N_COLUMNS
    };

////////////////////////////////////////////////////////////////////////

    ColormapProvider::Ptr ColormapProvider::create(PresentationInterface::Ptr p)
    {
      Colormappable::Ptr c = boost::dynamic_pointer_cast<Colormappable,
          PresentationInterface>(p);
      ColormapProvider::Ptr result;
      if (c)
      {
        result = ColormapProvider::Ptr(new ColormapProvider(c));

        Scroom::Utils::Stuff r = p->registerStrongObserver(result);
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
      std::list<Colormap::ConstPtr> maps =
          Colormaps::getInstance().getColormaps();

      colormaps = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);

      {
        // Add the original colormap, if any
        Colormap::Ptr orig = c->getOriginalColormap();
        if (orig)
        {
          Colormap::ConstPtr* cc = new Colormap::ConstPtr(orig);
          GtkTreeIter iter;
          gtk_list_store_append(colormaps, &iter);
          gtk_list_store_set(colormaps, &iter, COLUMN_NAME, (*cc)->name.c_str(),
              COLUMN_POINTER, cc, -1);
        }
      }

      while (!maps.empty())
      {
        if (maps.front()->colors.size() >= numColors)
        {
          Colormap::ConstPtr* cc = new Colormap::ConstPtr(maps.front());

          GtkTreeIter iter;
          gtk_list_store_append(colormaps, &iter);
          gtk_list_store_set(colormaps, &iter, COLUMN_NAME, (*cc)->name.c_str(),
              COLUMN_POINTER, cc, -1);
        }

        maps.pop_front();
      }
    }

    ColormapProvider::~ColormapProvider()
    {
      // Clear out all the smart pointers
      printf("ColormapProvider: Destructing...\n");
      if (colormaps)
      {
        GtkTreeIter iter;
        while (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(colormaps), &iter))
        {
          gpointer* pointer = NULL;
          gtk_tree_model_get(GTK_TREE_MODEL(colormaps), &iter, COLUMN_POINTER,
              &pointer, -1);
          Colormap::Ptr* colormap = reinterpret_cast<Colormap::Ptr*>(pointer);
          delete colormap;
          gtk_list_store_remove(colormaps, &iter);
        }
        g_object_unref(colormaps);
      }
      colormaps = NULL;
    }

    void ColormapProvider::open(ViewInterface::WeakPtr vi)
    {
      ViewInterface::Ptr vil(vi);
      printf("ColormapProvider: Adding a view.\n");
      GtkTreeView* tv =
          GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(colormaps)));
      GtkCellRenderer* txt = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
      gtk_tree_view_insert_column_with_attributes(tv, -1, "Name", txt, "text",
          COLUMN_NAME, NULL);
      g_signal_connect(static_cast<gpointer>(tv), "cursor_changed",
          G_CALLBACK (::on_colormap_selected), this);
      views[vi] = tv;

      vil->addSideWidget("Colormap", GTK_WIDGET(tv));
    }

    void ColormapProvider::close(ViewInterface::WeakPtr vi)
    {
      printf("ColormapProvider: Removing a view.\n");
      views.erase(vi);
      if (views.empty())
      {
        printf("ColormapProvider: Last view has gone.\n");
      }
    }

    void ColormapProvider::on_colormap_selected(GtkTreeView* tv)
    {
      Colormappable::Ptr c = colormappable.lock();
      if (c)
      {
        GtkTreeSelection* ts = gtk_tree_view_get_selection(tv);
        GtkTreeIter iter;
        GtkTreeModel* model = NULL;
        bool selected = gtk_tree_selection_get_selected(ts, &model, &iter);
        if (selected)
        {
          if (gtk_list_store_iter_is_valid(colormaps, &iter))
          {
            gpointer* pointer = NULL;
            gtk_tree_model_get(GTK_TREE_MODEL(colormaps), &iter, COLUMN_POINTER,
                &pointer, -1);
            Colormap::Ptr& colormap = *reinterpret_cast<Colormap::Ptr*>(pointer);
            c->setColormap(colormap);
          }
        }
      }
      else
        printf("PANIC: Colormappable Presentation is gone??\n");
    }
  }
}
