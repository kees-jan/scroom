/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <scroom/roi.hh>
#include <scroom/utilities.hh>

class RoiUi : public Scroom::Utils::Base, public Viewable
{
public:
  typedef boost::shared_ptr<RoiUi> Ptr;

private:
  Scroom::Roi::List::Ptr list;
  GtkTreeModel* model;


  Scroom::Utils::StuffList stuff;

private:
  RoiUi();
  void init(std::string const& fileName, ScroomInterface::Ptr const& scroomInterface);

  static void addRoiItemsToModel(GtkTreeStore* store, GtkTreeIter* base, std::vector<Scroom::Roi::RoiItem> const& regions);
  void create_and_fill_model();
  GtkWidget *create_view_and_model();

public:
  static Ptr create(const std::string& fileName, ScroomInterface::Ptr const& scroomInterface);

  ~RoiUi();

  virtual void open(ViewInterface::WeakPtr vi);
  virtual void close(ViewInterface::WeakPtr vi);

};


