/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "roiui.hh"

#include <stdio.h>
#include <gtk/gtk.h>

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

enum
{
  COL_NAME = 0,
  COL_POINTER,
  NUM_COLS
} ;

namespace Detail
{
  class ReturnBase: public boost::static_visitor<Scroom::Roi::RoiBase const&>
  {
  public:
    Scroom::Roi::RoiBase const& operator()(Scroom::Roi::RoiBase const& item) const;
    Scroom::Roi::RoiBase const& operator()(Scroom::Roi::Rect const& rect) const;
  };

  Scroom::Roi::RoiBase const& ReturnBase::operator()(Scroom::Roi::RoiBase const& item) const
  {
    return item;
  }

  Scroom::Roi::RoiBase const& ReturnBase::operator()(Scroom::Roi::Rect const& rect) const
  {
    return rect;
  }
}

Scroom::Roi::RoiBase const& getBase(Scroom::Roi::RoiItem const& item)
{
  return boost::apply_visitor(Detail::ReturnBase(), item);
}

RoiUi::Ptr RoiUi::create(const std::string& fileName, ScroomInterface::Ptr const& scroomInterface)
{
  Ptr result = Ptr(new RoiUi());
  result->init(fileName, scroomInterface);

  return result;
}

RoiUi::RoiUi() :
    model(NULL)
{}

RoiUi::~RoiUi()
{
  if(model)
  {
    g_object_unref (model);
    model = NULL;
  }
}

void RoiUi::init(std::string const& fileName, ScroomInterface::Ptr const& scroomInterface)
{
  list = Scroom::Roi::parse(fileName);
  printf("Regions: %zu\n", list->regions.size());

  std::set<ViewObservable::Ptr> viewObservables = list->instantiate(scroomInterface, fileName);

  if(!viewObservables.empty() && !list->regions.empty())
  {
    for(auto const & viewObservable: viewObservables)
    {
      stuff.push_back(viewObservable->registerStrongObserver(shared_from_this<RoiUi>()));
    }
  }
}

void RoiUi::addRoiItemsToModel(GtkTreeStore* store, GtkTreeIter* base, std::vector<Scroom::Roi::RoiItem> const& regions)
{
  GtkTreeIter iter;

  for(auto const& item: regions)
  {
    Scroom::Roi::RoiBase const& r = getBase(item);

    gtk_tree_store_append (store, &iter, base);
    gtk_tree_store_set (store, &iter,
                        COL_NAME, r.str().c_str(),
                        COL_POINTER, &r,
                        -1);

    addRoiItemsToModel(store, &iter, r.children);
  }
}

void RoiUi::create_and_fill_model()
{
  GtkTreeStore  *store;

  store = gtk_tree_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_POINTER);

  addRoiItemsToModel(store, NULL, list->regions);

  model = GTK_TREE_MODEL(store);
}

GtkWidget* RoiUi::create_view_and_model ()
{
  GtkCellRenderer     *renderer;
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

  create_and_fill_model();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  return view;
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
