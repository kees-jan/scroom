/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "colormapprovider.hh"

#include <cstdio>
#include <list>

#include <spdlog/spdlog.h>

#include <glib.h>

#include <scroom/bookkeeping.hh>
#include <scroom/colormappable.hh>
#include <scroom/presentationinterface.hh>

#include "colormaps.hh"

////////////////////////////////////////////////////////////////////////

namespace
{
  void on_colormap_selected(GtkTreeView* tv, gpointer user_data)
  {
    auto* cmp = static_cast<Scroom::ColormapImpl::ColormapProvider*>(user_data);
    cmp->on_colormap_selected(tv);
  }
} // namespace
////////////////////////////////////////////////////////////////////////

namespace Scroom::ColormapImpl
{
  /** Names of the columns in the GtkListStore */
  enum
  {
    COLUMN_NAME,
    COLUMN_POINTER,
    N_COLUMNS
  };

  ////////////////////////////////////////////////////////////////////////

  ColormapProvider::Ptr ColormapProvider::create(const PresentationInterface::Ptr& p)
  {
    Colormappable::Ptr const c = boost::dynamic_pointer_cast<Colormappable, PresentationInterface>(p);
    ColormapProvider::Ptr    result;
    if(c)
    {
      result = ColormapProvider::Ptr(new ColormapProvider(c));

      Scroom::Utils::Stuff const r = p->registerStrongObserver(result);
      result->registration         = r;
    }
    else
    {
      defect_message("PANIC: Presentation doesn't implement Colormappable");
    }

    return result;
  }

  ColormapProvider::ColormapProvider(const Colormappable::Ptr& c)
    : colormappable(c)

  {
    unsigned const int            numColors = c->getNumberOfColors();
    std::list<Colormap::ConstPtr> maps      = Colormaps::getInstance().getColormaps();

    colormaps = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);

    {
      // Add the original colormap, if any
      Colormap::Ptr const orig = c->getOriginalColormap();
      if(orig)
      {
        auto*       cc = new Colormap::ConstPtr(orig);
        GtkTreeIter iter;
        gtk_list_store_append(colormaps, &iter);
        gtk_list_store_set(colormaps, &iter, COLUMN_NAME, (*cc)->name.c_str(), COLUMN_POINTER, cc, -1);
      }
    }

    while(!maps.empty())
    {
      if(maps.front()->colors.size() >= numColors)
      {
        auto* cc = new Colormap::ConstPtr(maps.front());

        GtkTreeIter iter;
        gtk_list_store_append(colormaps, &iter);
        gtk_list_store_set(colormaps, &iter, COLUMN_NAME, (*cc)->name.c_str(), COLUMN_POINTER, cc, -1);
      }

      maps.pop_front();
    }
  }

  ColormapProvider::~ColormapProvider()
  {
    // Clear out all the smart pointers
    spdlog::debug("ColormapProvider: Destructing...");
    if(colormaps)
    {
      GtkTreeIter iter;
      while(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(colormaps), &iter))
      {
        gpointer* pointer = nullptr;
        gtk_tree_model_get(GTK_TREE_MODEL(colormaps), &iter, COLUMN_POINTER, &pointer, -1);
        auto* colormap = reinterpret_cast<Colormap::Ptr*>(pointer);
        delete colormap;
        gtk_list_store_remove(colormaps, &iter);
      }
      g_object_unref(colormaps);
    }
    colormaps = nullptr;
  }

  void ColormapProvider::open(ViewInterface::WeakPtr vi)
  {
    ViewInterface::Ptr const vil(vi);
    spdlog::debug("ColormapProvider: Adding a view.");
    GtkTreeView*     tv  = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(colormaps)));
    GtkCellRenderer* txt = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
    gtk_tree_view_insert_column_with_attributes(tv, -1, "Name", txt, "text", COLUMN_NAME, NULL);
    g_signal_connect(static_cast<gpointer>(tv), "cursor_changed", G_CALLBACK(::on_colormap_selected), this);
    views[vi] = tv;

    vil->addSideWidget("Colormap", GTK_WIDGET(tv));
  }

  void ColormapProvider::close(ViewInterface::WeakPtr vi)
  {
    spdlog::debug("ColormapProvider: Removing a view.");
    views.erase(vi);
    if(views.empty())
    {
      spdlog::debug("ColormapProvider: Last view has gone.");
    }
  }

  void ColormapProvider::on_colormap_selected(GtkTreeView* tv)
  {
    Colormappable::Ptr const c = colormappable.lock();
    if(c)
    {
      GtkTreeSelection* ts = gtk_tree_view_get_selection(tv);
      GtkTreeIter       iter;
      GtkTreeModel*     model    = nullptr;
      bool const        selected = gtk_tree_selection_get_selected(ts, &model, &iter);
      if(selected)
      {
        if(gtk_list_store_iter_is_valid(colormaps, &iter))
        {
          gpointer* pointer = nullptr;
          gtk_tree_model_get(GTK_TREE_MODEL(colormaps), &iter, COLUMN_POINTER, &pointer, -1);
          Colormap::Ptr const& colormap = *reinterpret_cast<Colormap::Ptr*>(pointer);
          c->setColormap(colormap);
        }
      }
    }
    else
    {
      defect_message("PANIC: Colormappable Presentation is gone??\n");
    }
  }
} // namespace Scroom::ColormapImpl
