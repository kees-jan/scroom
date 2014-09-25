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
  static Ptr create();
};

BOOST_AUTO_TEST_SUITE(Roi_Tests)

BOOST_AUTO_TEST_CASE(Parse_files)
{
  std::stringstream ss;
  ss << " * File: a.tif" << std::endl;
  ss << " * File: b.tif" << std::endl;
  ss << " * Aggregate: q" << std::endl;
  ss << "   * File: c.tif" << std::endl;
  ss << "   * File: d.tif" << std::endl;

  Roi::List l = Roi::parse(ss);
  // ScroomInterfaceStub::Ptr stub = ScroomInterfaceStub::create();

  // l.instantiate();

  UNUSED(l);
  // std::list<Roi::Presentations> presentations = list.presentations();

  BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
