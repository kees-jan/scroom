/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/test/unit_test.hpp>

#include <scroom/progressinterfacehelpers.hh>
#include <scroom/assertions.hh>

#include "progressstateinterfacestub.hh"

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(ProgressInterfaceBroadcaster_Tests)

BOOST_AUTO_TEST_CASE(each_child_is_notified)
{
  ProgressInterfaceBroadcaster::Ptr progressBroadcaster = ProgressInterfaceBroadcaster::create();
  BOOST_CHECK(progressBroadcaster);

  ProgressStateInterfaceStub::Ptr stub1 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub1);
  Stuff r1 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub1));
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub1->state);

  ProgressStateInterfaceStub::Ptr stub2 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub2);
  Stuff r2 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub2));
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub2->state);

  progressBroadcaster->setWorking(0.0);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub2->state);
  BOOST_CHECK_EQUAL(0.0, stub1->progress);
  BOOST_CHECK_EQUAL(0.0, stub2->progress);

  progressBroadcaster->setWaiting(0.25);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub2->state);
  BOOST_CHECK_EQUAL(0.25, stub1->progress);
  BOOST_CHECK_EQUAL(0.25, stub2->progress);

  progressBroadcaster->setWorking(0.5);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub2->state);
  BOOST_CHECK_EQUAL(0.5, stub1->progress);
  BOOST_CHECK_EQUAL(0.5, stub2->progress);

  progressBroadcaster->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::FINISHED, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::FINISHED, stub2->state);

  progressBroadcaster->setIdle();
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub2->state);
}

BOOST_AUTO_TEST_CASE(late_children_receive_current_state)
{
  ProgressInterfaceBroadcaster::Ptr progressBroadcaster = ProgressInterfaceBroadcaster::create();
  BOOST_CHECK(progressBroadcaster);

  progressBroadcaster->setWorking(0.0);

  ProgressStateInterfaceStub::Ptr stub1 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub1);
  Stuff r1 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub1));
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(0.0, stub1->progress);

  progressBroadcaster->setWaiting(0.5);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub1->state);
  BOOST_CHECK_EQUAL(0.5, stub1->progress);

  ProgressStateInterfaceStub::Ptr stub2 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub2);
  Stuff r2 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub2));

  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub2->state);
  BOOST_CHECK_EQUAL(0.5, stub2->progress);
}

BOOST_AUTO_TEST_CASE(unsubscribed_children_stop_receiving_events)
{
  ProgressInterfaceBroadcaster::Ptr progressBroadcaster = ProgressInterfaceBroadcaster::create();
  BOOST_CHECK(progressBroadcaster);

  ProgressStateInterfaceStub::Ptr stub1 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub1);
  Stuff r1 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub1));
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub1->state);

  ProgressStateInterfaceStub::Ptr stub2 = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub2);
  Stuff r2 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub2));
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub2->state);

  progressBroadcaster->setWorking(0.0);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub2->state);
  BOOST_CHECK_EQUAL(0.0, stub1->progress);
  BOOST_CHECK_EQUAL(0.0, stub2->progress);

  r1.reset();

  progressBroadcaster->setWaiting(0.25);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(0.0, stub1->progress);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub2->state);
  BOOST_CHECK_EQUAL(0.25, stub2->progress);

  r2.reset();

  progressBroadcaster->setWorking(0.5);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub1->state);
  BOOST_CHECK_EQUAL(0.0, stub1->progress);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub2->state);
  BOOST_CHECK_EQUAL(0.25, stub2->progress);
}

BOOST_AUTO_TEST_SUITE_END()
