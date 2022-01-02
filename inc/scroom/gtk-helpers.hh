/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <future>
#include <ostream>

#include <boost/function.hpp>

#include <gtk/gtk.h>

#include "assertions.hh"

namespace Scroom::GtkHelpers
{
  namespace Detail
  {
    template <typename T>
    static int gtkWrapper(gpointer data)
    {
      auto* w = reinterpret_cast<T*>(data);
      (*w)();
      delete w;
      return false;
    }
  } // namespace Detail

  template <typename T>
  std::pair<GSourceFunc, gpointer> wrap(T f)
  {
    return std::make_pair<GSourceFunc, gpointer>(Detail::gtkWrapper<T>, new T(std::move(f)));
  }

  bool on_ui_thread();

  template <typename T>
  void async_on_ui_thread(T f)
  {
    auto w = wrap(std::move(f));
    gdk_threads_add_idle(w.first, w.second);
  }

  template <typename T>
  void async_on_ui_thread_and_wait(T f)
  {
    require(!on_ui_thread());
    std::packaged_task<void(void)> t(f);
    std::future<void>              future = t.get_future();
    async_on_ui_thread(std::move(t));
    future.wait();
  }

  template <typename T>
  void sync_on_ui_thread(T f)
  {
    if(on_ui_thread())
    {
      f();
    }
    else
    {
      async_on_ui_thread_and_wait(std::move(f));
    }
  }

  inline cairo_rectangle_int_t createCairoIntRectangle(int x, int y, int width, int height)
  {
    cairo_rectangle_int_t rect;
    rect.x      = x;
    rect.y      = y;
    rect.width  = width;
    rect.height = height;
    return rect;
  }

  GtkWindow* get_parent_window(GtkWidget* widget);

} // namespace Scroom::GtkHelpers

inline bool operator==(cairo_rectangle_int_t const& left, cairo_rectangle_int_t const& right)
{
  return left.x == right.x && left.y == right.y && left.width == right.width && left.height == right.height;
}

inline bool operator==(GdkPoint const& left, GdkPoint const& right) { return left.x == right.x && left.y == right.y; }

std::ostream& operator<<(std::ostream& os, cairo_rectangle_int_t const& r);
std::ostream& operator<<(std::ostream& os, GdkPoint const& p);
