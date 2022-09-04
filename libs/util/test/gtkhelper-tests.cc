/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/weak_ptr.hpp>

#include <gtk/gtk.h>

#include <scroom/gtk-helpers.hh>
#include <scroom/gtk-test-helpers.hh>

using namespace Scroom::GtkHelpers;

namespace
{
  class B
  {
  public:
    using Ptr     = boost::shared_ptr<B>;
    using WeakPtr = boost::weak_ptr<B>;

    static Ptr create() { return Ptr(new B()); }
  };
} // namespace

static void b(const B::Ptr& /*unused*/) {}

BOOST_AUTO_TEST_SUITE(Gtk_Helpers_Tests)

BOOST_AUTO_TEST_CASE(function_returning_bool)
{
  GSourceFunc f    = nullptr;
  gpointer    data = nullptr;
  B::WeakPtr  wb;

  {
    B::Ptr const sb                          = B::create();
    wb                                       = sb;
    std::pair<GSourceFunc, gpointer> const w = wrap(boost::bind(b, sb));
    f                                        = w.first;
    data                                     = w.second;
  }
  BOOST_CHECK(wb.lock());

  bool const result = (*f)(data);
  BOOST_CHECK_EQUAL(false, result);
  BOOST_CHECK(!wb.lock());
}

BOOST_AUTO_TEST_CASE(on_ui_thread_test)
{
  Scroom::GtkTestHelpers::GtkMainLoop const mainLoop;

  BOOST_REQUIRE(!on_ui_thread());
  sync_on_ui_thread([] { BOOST_CHECK(on_ui_thread()); });
}

BOOST_AUTO_TEST_SUITE_END()
