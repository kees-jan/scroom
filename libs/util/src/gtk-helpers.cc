/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/thread.hpp>

#include <scroom/assertions.hh>
#include <scroom/gtk-helpers.hh>

namespace Scroom::GtkHelpers
{
  namespace Detail
  {
    boost::recursive_mutex& GdkMutex()
    {
      static boost::recursive_mutex me;
      return me;
    }

    void lockGdkMutex() { GdkMutex().lock(); }

    void unlockGdkMutex() { GdkMutex().unlock(); }
  } // namespace Detail

  void useRecursiveGdkLock() { gdk_threads_set_lock_functions(&Detail::lockGdkMutex, &Detail::unlockGdkMutex); }

  TakeGdkLock::TakeGdkLock() { gdk_threads_enter(); }

  TakeGdkLock::~TakeGdkLock() { gdk_threads_leave(); }
} // namespace Scroom::GtkHelpers

std::ostream& operator<<(std::ostream& os, cairo_rectangle_int_t const& r)
{
  return os << "GdkRectangle(" << r.x << ", " << r.y << ", " << r.width << ", " << r.height << ")";
}

std::ostream& operator<<(std::ostream& os, GdkPoint const& p) { return os << "GdkPoint(" << p.x << ", " << p.y << ")"; }
