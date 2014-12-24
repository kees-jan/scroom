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

#ifndef ROIUI_HH
#define ROIUI_HH

#include <boost/shared_ptr.hpp>

#include <scroom/roi.hh>
#include <scroom/utilities.hh>

class RoiUi : public Scroom::Utils::Base, public Scroom::Utils::Counted<RoiUi>, public Viewable
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

#endif
