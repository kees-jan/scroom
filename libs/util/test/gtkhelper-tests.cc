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
#include <scroom/gtk-helpers.hh>

#include <gtk/gtk.h>

#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/bind.hpp>

using namespace Scroom::GtkHelpers;

namespace
{
  class B
  {
  public:
    typedef boost::shared_ptr<B> Ptr;
    typedef boost::weak_ptr<B> WeakPtr;

    static Ptr create() { return Ptr(new B()); }
  };
}

static bool b(bool& in, B::Ptr)
{
  return in;
}

BOOST_AUTO_TEST_SUITE(Gtk_Helpers_Tests)

BOOST_AUTO_TEST_CASE(function_returning_bool)
{
  bool in=true;
  GtkFunction f = NULL;
  gpointer data = NULL;
  B::WeakPtr wb;

  {
    B::Ptr sb = B::create();
    wb = sb;
    Wrapper w = wrap(boost::bind(b,boost::ref(in),sb));
    f = w.f;
    data = w.data;
  }
  BOOST_CHECK(wb.lock());

  bool result = (*f)(data);
  BOOST_CHECK_EQUAL(true, result);
  BOOST_CHECK(wb.lock());
  in=false;
  result = (*f)(data);
  BOOST_CHECK_EQUAL(false, result);
  BOOST_CHECK(!wb.lock());

}

BOOST_AUTO_TEST_SUITE_END()

