/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>
#include <boost/test/results_reporter.hpp>

#include <scroom/unused.h>

bool init_unit_test()
{
  return true;
}

int main( int argc, char* argv[] )
{
#ifdef XML_TEST_OUTPUT
  std::cerr << "You have requested XML output. Your command-line arguments will be ignored" << std::endl;
  UNUSED(argc);
  UNUSED(argv);

  // Apparently, order is important here. Weird but true...
  const char * alternative[] = {
    "--log_format=XML",
    "--log_level=all",
    "--log_sink=test_results.xml",
    "--output_format=XML",
    "--report_level=no",
  };
  int count = sizeof(alternative)/sizeof(alternative[0]);
  
  return ::boost::unit_test::unit_test_main( &init_unit_test, count, const_cast<char**>(alternative) );
#else
  return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
#endif
}
