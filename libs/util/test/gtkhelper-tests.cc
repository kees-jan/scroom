/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <memory>

#include <gtest/gtest.h>

#include <gtk/gtk.h>

#include <scroom/gtk-helpers.hh>
#include <scroom/gtk-test-helpers.hh>

using namespace Scroom::GtkHelpers;

namespace
{
  class B
  {
  public:
    using Ptr = std::shared_ptr<B>;
    using WeakPtr = std::weak_ptr<B>;

    static Ptr create() { return std::make_shared<B>(); }
  };
} // namespace

static void b(const B::Ptr& /*unused*/) {}

TEST(Gtk_Helpers_Tests, function_returning_bool) // NOLINT
{
  GSourceFunc f = nullptr;
  gpointer data = nullptr;
  B::WeakPtr wb;

  {
    B::Ptr const sb = B::create();
    wb = sb;
    std::pair<GSourceFunc, gpointer> const w = wrap([sb] { return b(sb); });
    f = w.first;
    data = w.second;
  }
  EXPECT_TRUE(wb.lock());

  bool const result = (*f)(data);
  EXPECT_EQ(false, result);
  EXPECT_FALSE(wb.lock());
}

TEST(Gtk_Helpers_Tests, on_ui_thread_test) // NOLINT
{
  Scroom::GtkTestHelpers::GtkMainLoop const mainLoop;

  ASSERT_FALSE(on_ui_thread());
  sync_on_ui_thread([] { EXPECT_TRUE(on_ui_thread()); });
}
