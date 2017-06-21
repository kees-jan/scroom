/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

//////////////////////////////////////////////////////////////

namespace
{
  void do_nothing()
  {}
}

BOOST_AUTO_TEST_SUITE(Packaged_tasks)

BOOST_AUTO_TEST_CASE(abandoned_tasks_return_exceptions)
{
  boost::unique_future<void> f;
  BOOST_CHECK(!f.is_ready());
  BOOST_CHECK(!f.has_exception());
  BOOST_CHECK(!f.has_value());
  
  {
    boost::packaged_task<void> t(do_nothing);
    f = t.get_future();
  }

  // At this point, the future should have an exception
  BOOST_CHECK(f.is_ready());
  BOOST_CHECK(f.has_exception());
  BOOST_CHECK(!f.has_value());

  f.wait(); // returns immediately, no exception

  BOOST_CHECK_THROW(f.get(), boost::broken_promise);
}

BOOST_AUTO_TEST_SUITE_END()
