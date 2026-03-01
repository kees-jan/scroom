/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gtest/gtest.h>

#include <scroom/assertions.hh>
#include <scroom/progressinterfacehelpers.hh>

#include "progressstateinterfacestub.hh"

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

class ProgressInterfaceMultiplexer_Tests : public ::testing::Test
{
public:
  ProgressStateInterfaceStub::Ptr stub;
  ProgressInterface::Ptr          p1;
  ProgressInterface::Ptr          p2;

public:
  ProgressInterfaceMultiplexer_Tests()
  {
    stub = ProgressStateInterfaceStub::create();
    EXPECT_TRUE(stub);
    EXPECT_EQ(ProgressStateInterface::IDLE, stub->state);
    ProgressInterface::Ptr const parent = ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub);

    ProgressInterfaceMultiplexer::Ptr const multiplexer = ProgressInterfaceMultiplexer::create(parent);
    EXPECT_TRUE(multiplexer);
    EXPECT_EQ(ProgressStateInterface::IDLE, stub->state);

    p1 = multiplexer->createProgressInterface();
    p2 = multiplexer->createProgressInterface();
  }
};

//////////////////////////////////////////////////////////////

TEST_F(ProgressInterfaceMultiplexer_Tests, each_subinterface_contributes_proportionally) // NOLINT
{
  p1->setWorking(0.0);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.0, stub->progress);
  p2->setWorking(0.2);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.1, stub->progress);
  p2->setFinished();
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.5, stub->progress);
  p1->setFinished();
  EXPECT_EQ(ProgressStateInterface::FINISHED, stub->state);
}

TEST_F(ProgressInterfaceMultiplexer_Tests, idle_subinterfaces_dont_count) // NOLINT
{
  p1->setIdle();
  p2->setWorking(0.2);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.2, stub->progress);

  p1->setWorking(0.0);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.1, stub->progress);

  p1->setIdle();
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.2, stub->progress);

  p2->setFinished();
  EXPECT_EQ(ProgressStateInterface::FINISHED, stub->state);
}

TEST_F(ProgressInterfaceMultiplexer_Tests, finishing_the_last_interface_resets_progress) // NOLINT
{
  p1->setIdle();
  p2->setWorking(0.2);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.2, stub->progress);

  p2->setFinished();
  EXPECT_EQ(ProgressStateInterface::FINISHED, stub->state);

  p1->setWorking(0.2);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.2, stub->progress);
}

TEST_F(ProgressInterfaceMultiplexer_Tests, disappearing_clients_dont_count_any_more) // NOLINT
{
  p1->setWorking(0.0);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.0, stub->progress);
  p2->setWorking(0.2);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.1, stub->progress);
  p1.reset();
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.2, stub->progress);
}

TEST_F(ProgressInterfaceMultiplexer_Tests, waiting_plus_working_equals_working) // NOLINT
{
  p1->setWaiting();
  p2->setWorking(0.0);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.0, stub->progress);
}

TEST_F(ProgressInterfaceMultiplexer_Tests, waiting_plus_finished_equals_waiting) // NOLINT
{
  p1->setWaiting();
  p2->setFinished();
  EXPECT_EQ(ProgressStateInterface::WAITING, stub->state);
  EXPECT_EQ(0.5, stub->progress);
}

TEST_F(ProgressInterfaceMultiplexer_Tests, working_plus_finished_equals_working) // NOLINT
{
  p1->setWorking(0.0);
  p2->setFinished();
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.5, stub->progress);
}

TEST_F(ProgressInterfaceMultiplexer_Tests, working_plus_waiting_equals_working) // NOLINT
{
  p1->setWorking(0.0);
  p2->setWaiting();
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.0, stub->progress);
}

TEST_F(ProgressInterfaceMultiplexer_Tests, finished_plus_waiting_equals_waiting) // NOLINT
{
  p2->setWaiting();
  p1->setFinished();
  EXPECT_EQ(ProgressStateInterface::WAITING, stub->state);
  EXPECT_EQ(0.5, stub->progress);
}

TEST_F(ProgressInterfaceMultiplexer_Tests, finished_plus_working_equals_working) // NOLINT
{
  p2->setWorking(0.0);
  p1->setFinished();
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.5, stub->progress);
}
