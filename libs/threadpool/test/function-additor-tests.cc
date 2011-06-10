/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <boost/test/unit_test.hpp>

#include <scroom/semaphore.hh>

#include "function-additor.hh"
#include "helpers.hh"

using namespace Scroom;

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Function_Additor_Tests)

BOOST_AUTO_TEST_SUITE(Helper_Tests)

BOOST_AUTO_TEST_CASE(clear_clears)
{
  Semaphore s(0);
  BOOST_REQUIRE(!s.try_P());
  clear(&s)();
  BOOST_REQUIRE(s.try_P());
}

BOOST_AUTO_TEST_CASE(pass_passes)
{
  Semaphore s(1);
  pass(&s)();
  BOOST_REQUIRE(!s.try_P());
}

BOOST_AUTO_TEST_CASE(destroy_destroys)
{
  boost::shared_ptr<int> p(new int(4));
  boost::weak_ptr<int> w(p);
  BOOST_REQUIRE(w.lock());

  boost::function<void ()> f(destroy(p));
  p.reset();
  BOOST_REQUIRE(w.lock());

  f();
  BOOST_REQUIRE(!w.lock());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Additor_Tests)

BOOST_AUTO_TEST_CASE(additor_adds)
{
  Semaphore s1(0);
  Semaphore s2(0);
  BOOST_REQUIRE(!s1.try_P());
  BOOST_REQUIRE(!s2.try_P());
  (clear(&s1) + clear(&s2))();
  BOOST_REQUIRE(s1.try_P());
  BOOST_REQUIRE(s2.try_P());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
