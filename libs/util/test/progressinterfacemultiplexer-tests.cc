/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/test/unit_test.hpp>

#include <scroom/assertions.hh>
#include <scroom/progressinterfacehelpers.hh>

#include "progressstateinterfacestub.hh"

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

class ProgressInterfaceMultiplexer_Fixture
{
public:
  ProgressStateInterfaceStub::Ptr stub;
  ProgressInterface::Ptr          p1;
  ProgressInterface::Ptr          p2;

public:
  ProgressInterfaceMultiplexer_Fixture();
};

ProgressInterfaceMultiplexer_Fixture::ProgressInterfaceMultiplexer_Fixture()
{
  stub = ProgressStateInterfaceStub::create();
  BOOST_CHECK(stub);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);
  ProgressInterface::Ptr parent = ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub);

  ProgressInterfaceMultiplexer::Ptr multiplexer = ProgressInterfaceMultiplexer::create(parent);
  BOOST_CHECK(multiplexer);
  BOOST_CHECK_EQUAL(ProgressStateInterface::IDLE, stub->state);

  p1 = multiplexer->createProgressInterface();
  p2 = multiplexer->createProgressInterface();
}

//////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE(ProgressInterfaceMultiplexer_Tests, ProgressInterfaceMultiplexer_Fixture)

BOOST_AUTO_TEST_CASE(each_subinterface_contributes_proportionally)
{
  p1->setWorking(0.0);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.0, stub->progress);
  p2->setWorking(0.2);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.1, stub->progress);
  p2->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.5, stub->progress);
  p1->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::FINISHED, stub->state);
}

BOOST_AUTO_TEST_CASE(idle_subinterfaces_dont_count)
{
  p1->setIdle();
  p2->setWorking(0.2);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.2, stub->progress);

  p1->setWorking(0.0);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.1, stub->progress);

  p1->setIdle();
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.2, stub->progress);

  p2->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::FINISHED, stub->state);
}

BOOST_AUTO_TEST_CASE(finishing_the_last_interface_resets_progress)
{
  p1->setIdle();
  p2->setWorking(0.2);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.2, stub->progress);

  p2->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::FINISHED, stub->state);

  p1->setWorking(0.2);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.2, stub->progress);
}

BOOST_AUTO_TEST_CASE(disappearing_clients_dont_count_any_more)
{
  p1->setWorking(0.0);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.0, stub->progress);
  p2->setWorking(0.2);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.1, stub->progress);
  p1.reset();
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.2, stub->progress);
}

BOOST_AUTO_TEST_CASE(waiting_plus_working_equals_working)
{
  p1->setWaiting();
  p2->setWorking(0.0);
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.0, stub->progress);
}

BOOST_AUTO_TEST_CASE(waiting_plus_finished_equals_waiting)
{
  p1->setWaiting();
  p2->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub->state);
  BOOST_CHECK_EQUAL(0.5, stub->progress);
}

BOOST_AUTO_TEST_CASE(working_plus_finished_equals_working)
{
  p1->setWorking(0.0);
  p2->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.5, stub->progress);
}

BOOST_AUTO_TEST_CASE(working_plus_waiting_equals_working)
{
  p1->setWorking(0.0);
  p2->setWaiting();
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.0, stub->progress);
}

BOOST_AUTO_TEST_CASE(finished_plus_waiting_equals_waiting)
{
  p2->setWaiting();
  p1->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::WAITING, stub->state);
  BOOST_CHECK_EQUAL(0.5, stub->progress);
}

BOOST_AUTO_TEST_CASE(finished_plus_working_equals_working)
{
  p2->setWorking(0.0);
  p1->setFinished();
  BOOST_CHECK_EQUAL(ProgressStateInterface::WORKING, stub->state);
  BOOST_CHECK_EQUAL(0.5, stub->progress);
}

BOOST_AUTO_TEST_SUITE_END()
