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

#include <iostream>
#include <fstream>
#include <cassert>

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>
#include <boost/test/results_reporter.hpp>

#ifdef XML_TEST_OUTPUT
std::ofstream out;
#endif


bool init_unit_test()
{
#ifdef XML_TEST_OUTPUT
  out.open("test_results.xml");
  assert(out.is_open());
  boost::unit_test::results_reporter::set_format(boost::unit_test::XML);
  boost::unit_test::results_reporter::set_level(boost::unit_test::DETAILED_REPORT);
  boost::unit_test::results_reporter::set_stream(out);
#endif

  return true;
}

int main( int argc, char* argv[] )
{
  return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
