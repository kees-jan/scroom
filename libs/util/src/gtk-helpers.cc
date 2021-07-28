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

  namespace
  {
    std::optional<std::thread::id> ui_thread; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
  }

  void this_is_the_ui_thread() { ui_thread = std::this_thread::get_id(); }

  bool on_ui_thread()
  {
    require(ui_thread);

    return *ui_thread == std::this_thread::get_id();
  }

  void useRecursiveGdkLock() { gdk_threads_set_lock_functions(&Detail::lockGdkMutex, &Detail::unlockGdkMutex); }
} // namespace Scroom::GtkHelpers

std::ostream& operator<<(std::ostream& os, cairo_rectangle_int_t const& r)
{
  return os << "GdkRectangle(" << r.x << ", " << r.y << ", " << r.width << ", " << r.height << ")";
}

std::ostream& operator<<(std::ostream& os, GdkPoint const& p) { return os << "GdkPoint(" << p.x << ", " << p.y << ")"; }
