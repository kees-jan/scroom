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
  bool on_ui_thread() { return g_main_context_is_owner(g_main_context_default()); }

  GtkWindow* get_parent_window(GtkWidget* widget)
  {
    auto* topLevel = gtk_widget_get_toplevel(widget);
    if(GTK_IS_WINDOW(topLevel))
    {
      return GTK_WINDOW(topLevel);
    }
    else
    {
      return nullptr;
    }
  }
} // namespace Scroom::GtkHelpers

std::ostream& operator<<(std::ostream& os, cairo_rectangle_int_t const& r)
{
  return os << "cairo_rectangle_int_t(" << r.x << ", " << r.y << ", " << r.width << ", " << r.height << ")";
}

std::ostream& operator<<(std::ostream& os, GdkPoint const& p) { return os << "GdkPoint(" << p.x << ", " << p.y << ")"; }
