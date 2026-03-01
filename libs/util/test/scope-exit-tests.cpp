/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gtest/gtest.h>

#include <scroom/utilities.hh>

using namespace Scroom::Utils;

TEST(scope_exit_tests, test_on_scope_exit) // NOLINT
{
  bool result = false;
  {
    on_scope_exit set_result_to_true([&] { result = true; });
    EXPECT_FALSE(result);
  }
  EXPECT_TRUE(result);
}

TEST(scope_exit_tests, test_on_destruction) // NOLINT
{
  bool result = false;
  auto s      = on_destruction([&] { result = true; });
  EXPECT_FALSE(result);
  s.reset();
  EXPECT_TRUE(result);
}

TEST(scope_exit_tests, test_desired_optional_cleanup) // NOLINT
{
  bool result = false;
  {
    optional_cleanup set_result_to_true([&] { result = true; });
    EXPECT_FALSE(result);
  }
  EXPECT_TRUE(result);
}

TEST(scope_exit_tests, test_undesired_optional_cleanup) // NOLINT
{
  bool result = false;
  {
    optional_cleanup set_result_to_true([&] { result = true; });
    EXPECT_FALSE(result);
    set_result_to_true.cancel();
  }
  EXPECT_FALSE(result);
}
