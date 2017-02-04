/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/test/unit_test.hpp>

#include <scroom/progressinterfacehelpers.hh>
#include <scroom/assertions.hh>

#include "progressstateinterfacestub.hh"

using namespace Scroom::Utils;

BOOST_AUTO_TEST_SUITE(test_converters)

BOOST_AUTO_TEST_CASE(test_ProgressInterfaceFromProgressStateInterface)
{
  ProgressStateInterfaceStub::Ptr stub = ProgressStateInterfaceStub::create();
  BOOST_REQUIRE(stub);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);

  ProgressInterface::Ptr pi = ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub);
  BOOST_REQUIRE(pi);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);

  pi->setWaiting();
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub->state);
  BOOST_CHECK_EQUAL(0.0, stub->progress);
  
  pi->setWorking(0.33);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.33, stub->progress);

  pi->setWaiting(0.25);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub->state);
  BOOST_CHECK_EQUAL(0.25, stub->progress);
  
  pi->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::FINISHED, stub->state);
  BOOST_CHECK_EQUAL(1.0, stub->progress);
  
  pi->setWorking(0.75);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.75, stub->progress);
  
  pi->setIdle();
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);
}

BOOST_AUTO_TEST_CASE(test_ProgressStateInterfaceFromProgressInterface)
{
  ProgressStateInterfaceStub::Ptr stub = ProgressStateInterfaceStub::create();
  BOOST_REQUIRE(stub);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);

  ProgressInterface::Ptr pi = ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub);
  BOOST_REQUIRE(pi);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);

  ProgressStateInterface::Ptr ps = ProgressStateInterfaceFromProgressInterfaceForwarder::create(pi);
  BOOST_REQUIRE(ps);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);

  stub->progress = 0.33;

  ps->setProgress(ProgressStateInterface::WAITING);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub->state);
  BOOST_CHECK_EQUAL(0.0, stub->progress);
  
  ps->setProgress(ProgressStateInterface::WORKING, 0.27);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.27, stub->progress);

  ps->setProgress(ProgressStateInterface::WAITING, 0.27);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub->state);
  BOOST_CHECK_EQUAL(0.27, stub->progress);
  
  ps->setProgress(ProgressStateInterface::FINISHED);
  BOOST_CHECK_EQUAL(ProgressStateInterface::FINISHED, stub->state);
  BOOST_CHECK_EQUAL(1.0, stub->progress);
  
  ps->setProgress(ProgressStateInterface::WORKING, 0.75);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.75, stub->progress);
  
  ps->setProgress(ProgressStateInterface::IDLE);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);
}

BOOST_AUTO_TEST_SUITE_END()
