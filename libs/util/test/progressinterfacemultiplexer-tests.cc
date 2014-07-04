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

#include <boost/test/unit_test.hpp>

#include <scroom/progressinterfacemultiplexer.hh>
#include <scroom/assertions.hh>

#include "progressstateinterfacestub.hh"

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(ProgressInterfaceMultiplexer_Tests)

BOOST_AUTO_TEST_CASE(each_subinterface_contributes_proportionally)
{
  ProgressStateInterfaceStub::Ptr stub = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);

  ProgressInterfaceMultiplexer::Ptr multiplexer = ProgressInterfaceMultiplexer::create(stub);
  BOOST_CHECK(multiplexer);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);

  ProgressStateInterface::Ptr p1 = multiplexer->createProgressInterface();
  ProgressStateInterface::Ptr p2 = multiplexer->createProgressInterface();

  p1->setProgress(ProgressStateInterface::WORKING);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.0, stub->progress);
  p2->setProgress(ProgressStateInterface::WORKING);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.0, stub->progress);
  p1->setProgress(ProgressStateInterface::FINISHED);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.5, stub->progress);
  p2->setProgress(ProgressStateInterface::FINISHED);
  BOOST_CHECK_EQUAL(ProgressStateInterface::FINISHED, stub->state);
}

BOOST_AUTO_TEST_SUITE_END()
