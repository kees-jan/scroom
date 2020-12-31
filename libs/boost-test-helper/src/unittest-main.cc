/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>
#include <sstream>
#include <string>

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/results_reporter.hpp>
#include <boost/test/unit_test.hpp>

#include <scroom/unused.hh>

bool init_unit_test() { return true; }

std::string extract_path(const std::string& cmd)
{
  size_t n = cmd.find_last_of('/');
  if(n == std::string::npos)
  {
    return "";
  }
  else
  {
    return cmd.substr(0, n);
  }
}

int main(int argc, char* argv[])
{
#ifdef XML_TEST_OUTPUT
  if(argc > 1)
    std::cerr << "You have requested XML output. Your command-line arguments will be ignored" << std::endl;

  std::string       path = extract_path(argv[0]);
  std::stringstream outputArgument;
  outputArgument << "--log_sink=";
  if(!path.empty())
    outputArgument << path << '/';
  outputArgument << "test_results.xml";

  std::string outputArgumentString = outputArgument.str();

  // Apparently, order is important here. Weird but true...
  const char* alternative[] = {
    "--log_format=XML",
    "--log_level=all",
    outputArgumentString.c_str(),
    "--output_format=XML",
    "--report_level=no",
  };
  int count = sizeof(alternative) / sizeof(alternative[0]);

  return ::boost::unit_test::unit_test_main(&init_unit_test, count, const_cast<char**>(alternative));
#else
  return ::boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
#endif
}
