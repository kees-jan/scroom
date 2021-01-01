/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <ostream>

#include <boost/function.hpp>

#include <gtk/gtk.h>

namespace Scroom
{
  namespace GtkHelpers
  {
    class Wrapper
    {
    public:
      GtkFunction f;
      gpointer    data;

    public:
      Wrapper(const boost::function<bool()>& f);
    };

    Wrapper wrap(boost::function<bool()> f);

    class TakeGdkLock
    {
    public:
      TakeGdkLock();
      TakeGdkLock(const TakeGdkLock&) = delete;
      TakeGdkLock(TakeGdkLock&&)      = delete;
      TakeGdkLock& operator=(const TakeGdkLock&) = delete;
      TakeGdkLock& operator=(TakeGdkLock&&) = delete;
      ~TakeGdkLock();
    };

    void useRecursiveGdkLock();

    inline GdkRectangle createGdkRectangle(int x, int y, int width, int height)
    {
      GdkRectangle rect;
      rect.x      = x;
      rect.y      = y;
      rect.width  = width;
      rect.height = height;
      return rect;
    }

  } // namespace GtkHelpers
} // namespace Scroom

inline bool operator==(GdkRectangle const& left, GdkRectangle const& right)
{
  return left.x == right.x && left.y == right.y && left.width == right.width && left.height == right.height;
}

inline bool operator==(GdkPoint const& left, GdkPoint const& right) { return left.x == right.x && left.y == right.y; }

std::ostream& operator<<(std::ostream& os, GdkRectangle const& r);
std::ostream& operator<<(std::ostream& os, GdkPoint const& p);
