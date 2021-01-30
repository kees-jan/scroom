/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <gtk/gtk.h>

#include <scroom/interface.hh>

class OpenTiledBitmapInterface : private Interface
{
public:
  using Ptr = boost::shared_ptr<OpenTiledBitmapInterface>;

public:
  virtual std::list<GtkFileFilter*> getFilters() = 0;

  virtual void open(const std::string& fileName) = 0;
};