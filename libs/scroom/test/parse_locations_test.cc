/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

#include <scroom/roi.hh>

#include <list>
#include <fstream>

#include <boost/test/unit_test.hpp>

#include <scroom/unused.hh>
#include <scroom/scroominterface.hh>

#include <roi-parser.hh>

namespace Roi = Scroom::Roi;

class ScroomInterfaceStub : public ScroomInterface
{
public:
  typedef boost::shared_ptr<ScroomInterfaceStub> Ptr;

public:
  std::set<std::string> openedFiles;

public:
  static Ptr create();
};

ScroomInterfaceStub::Ptr ScroomInterfaceStub::create()
{
  return Ptr(new ScroomInterfaceStub());
}

BOOST_AUTO_TEST_SUITE(Roi_Tests)

BOOST_AUTO_TEST_CASE(Parse_files)
{
  std::stringstream ss;
  ss << " * File: a.tif" << std::endl;
  ss << " * File: b.tif" << std::endl;
  ss << " * Aggregate: q" << std::endl;
  ss << "   * File: c.tif" << std::endl;
  ss << "   * File: d.tif" << std::endl;
  ss << "   * Aggregate: r" << std::endl;
  ss << "     * File: e.tif" << std::endl;
  ss << "   * File: f.tif" << std::endl;
  ss << " * File: g.tif" << std::endl;

  std::string input = ss.str();
  
  std::vector<Roi::Detail::Presentation> presentations = Roi::Detail::parse(input.begin(), input.end());

  // BOOST_FOREACH(Roi::Detail::Presentation const & p, presentations)
  //   std::cout << "Found presentation: " << p << std::endl;

  BOOST_CHECK_EQUAL(4, presentations.size());
}

// BOOST_AUTO_TEST_CASE(Parse_files2)
// {
//   std::stringstream ss;
//   ss << " * File: a.tif" << std::endl;
//   ss << " * File: b.tif" << std::endl;
//   ss << " * Aggregate: q" << std::endl;
//   ss << "   * File: c.tif" << std::endl;
//   ss << "   * File: d.tif" << std::endl;
// 
//   std::ofstream f("/tmp/testdata.txt");
//   f << ss.str();
//   
//   Roi::List l = Roi::parse(ss);
//   ScroomInterfaceStub::Ptr stub = ScroomInterfaceStub::create();
// 
//   std::set<ViewObserver::Ptr> presentations = l.instantiate(stub);
// 
//   BOOST_CHECK_EQUAL(4, stub->openedFiles.size());
// }

BOOST_AUTO_TEST_SUITE_END()
