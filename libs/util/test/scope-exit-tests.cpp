/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/test/unit_test.hpp>

#include <scroom/utilities.hh>

using Scroom::Utils::on_destruction;
using Scroom::Utils::on_scope_exit;

BOOST_AUTO_TEST_SUITE(scope_exit_tests)

BOOST_AUTO_TEST_CASE(test_on_scope_exit)
{
  bool result = false;
  {
    on_scope_exit set_result_to_true([&] { result = true; });
    BOOST_TEST(!result);
  }
  BOOST_TEST(result);
}

BOOST_AUTO_TEST_CASE(test_on_destruction)
{
  bool result = false;
  auto s      = on_destruction([&] { result = true; });
  BOOST_TEST(!result);
  s.reset();
  BOOST_TEST(result);
}

BOOST_AUTO_TEST_SUITE_END()
