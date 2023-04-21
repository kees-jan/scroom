/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>

bool init_unit_test() { return true; }

int main(int argc, char* argv[]) { return ::boost::unit_test::unit_test_main(&init_unit_test, argc, argv); }
