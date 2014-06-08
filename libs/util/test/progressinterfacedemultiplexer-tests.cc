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

#include <scroom/progressinterfacehelpers.hh>
#include <scroom/assertions.hh>

#include "progressstateinterfacestub.hh"

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(ProgressInterfaceDemultiplexer_Tests)

BOOST_AUTO_TEST_CASE(each_child_is_notified)
{
  ProgressInterfaceDemultiplexer::Ptr demultiplexer = ProgressInterfaceDemultiplexer::create();
  BOOST_CHECK(demultiplexer);

  ProgressStateInterfaceStub::Ptr stub1 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub1);
  Stuff r1 = demultiplexer->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub1));
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub1->state);

  ProgressStateInterfaceStub::Ptr stub2 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub2);
  Stuff r2 = demultiplexer->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub2));
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub2->state);

  demultiplexer->setWorking(0.0);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub2->state);
  BOOST_CHECK_EQUAL(0.0, stub1->progress);
  BOOST_CHECK_EQUAL(0.0, stub2->progress);

  demultiplexer->setWaiting(0.25);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub2->state);
  BOOST_CHECK_EQUAL(0.25, stub1->progress);
  BOOST_CHECK_EQUAL(0.25, stub2->progress);
  
  demultiplexer->setWorking(0.5);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub2->state);
  BOOST_CHECK_EQUAL(0.5, stub1->progress);
  BOOST_CHECK_EQUAL(0.5, stub2->progress);
  
  demultiplexer->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::FINISHED, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::FINISHED, stub2->state);

  demultiplexer->setIdle();
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub2->state);
}

BOOST_AUTO_TEST_CASE(late_children_receive_current_state)
{
  ProgressInterfaceDemultiplexer::Ptr demultiplexer = ProgressInterfaceDemultiplexer::create();
  BOOST_CHECK(demultiplexer);

  demultiplexer->setWorking(0.0);

  ProgressStateInterfaceStub::Ptr stub1 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub1);
  Stuff r1 = demultiplexer->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub1));
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(0.0, stub1->progress);

  demultiplexer->setWaiting(0.5);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub1->state);
  BOOST_CHECK_EQUAL(0.5, stub1->progress);

  ProgressStateInterfaceStub::Ptr stub2 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub2);
  Stuff r2 = demultiplexer->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub2));

  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub2->state);
  BOOST_CHECK_EQUAL(0.5, stub2->progress);
}

BOOST_AUTO_TEST_CASE(unsubscribed_children_stop_receiving_events)
{
  ProgressInterfaceDemultiplexer::Ptr demultiplexer = ProgressInterfaceDemultiplexer::create();
  BOOST_CHECK(demultiplexer);

  ProgressStateInterfaceStub::Ptr stub1 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub1);
  Stuff r1 = demultiplexer->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub1));
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub1->state);

  ProgressStateInterfaceStub::Ptr stub2 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub2);
  Stuff r2 = demultiplexer->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub2));
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub2->state);

  demultiplexer->setWorking(0.0);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub2->state);
  BOOST_CHECK_EQUAL(0.0, stub1->progress);
  BOOST_CHECK_EQUAL(0.0, stub2->progress);

  r1.reset();

  demultiplexer->setWaiting(0.25);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(0.0, stub1->progress);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub2->state);
  BOOST_CHECK_EQUAL(0.25, stub2->progress);

  r2.reset();
  
  demultiplexer->setWorking(0.5);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(0.0, stub1->progress);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub2->state);
  BOOST_CHECK_EQUAL(0.25, stub2->progress);
}

BOOST_AUTO_TEST_SUITE_END()
