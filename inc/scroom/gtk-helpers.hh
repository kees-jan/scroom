/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
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
#ifndef _GTK_HELPERS_H_
#define _GTK_HELPERS_H_

#include <ostream>

#include <gtk/gtk.h>

#include <boost/function.hpp>

namespace Scroom
{
  namespace GtkHelpers
  {
    class Wrapper
    {
    public:
      GtkFunction f;
      gpointer data;

    public:
      Wrapper(const boost::function<bool ()>& f);
    };

    Wrapper wrap(boost::function<bool ()> f);

    class TakeGdkLock
    {
    public:
      TakeGdkLock();
      ~TakeGdkLock();
    };

    void useRecursiveGdkLock();

    inline GdkRectangle createGdkRectangle(int x, int y, int width, int height)
    {
      GdkRectangle rect;
      rect.x = x;
      rect.y = y;
      rect.width = width;
      rect.height = height;
      return rect;
    }

  }
}

inline bool operator==(GdkRectangle const& left, GdkRectangle const& right)
{
  return
    left.x == right.x &&
    left.y == right.y &&
    left.width == right.width &&
    left.height == right.height;
}

std::ostream& operator<<(std::ostream& os, GdkRectangle const& r);

#endif


