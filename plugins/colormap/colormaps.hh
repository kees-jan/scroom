/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

#ifndef COLORMAPS_HH
#define COLORMAPS_HH

#include <gtk/gtk.h>

#include<colormappable.hh>

class Colormaps
{
private:
  GtkListStore* filenames;

private:
  Colormaps();
  ~Colormaps();

public:
  static Colormaps& getInstance();
  
  GtkListStore* getFileNames();

  void select(GtkTreeIter iter, Colormappable* colormappable);
  Colormap::Ptr load(const char* name);
};

enum
  {
    COLUMN_NAME,
    COLUMN_POINTER,
    COLUMN_ENABLED,
    N_COLUMNS
  };

#endif
