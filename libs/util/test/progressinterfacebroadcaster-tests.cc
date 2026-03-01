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

TEST(ProgressInterfaceBroadcaster_Tests, each_child_is_notified) // NOLINT
{
  ProgressInterfaceBroadcaster::Ptr const progressBroadcaster = ProgressInterfaceBroadcaster::create();
  EXPECT_TRUE(progressBroadcaster);

  ProgressStateInterfaceStub::Ptr const stub1 = ProgressStateInterfaceStub::create();
  EXPECT_TRUE(stub1);
  Stuff const r1 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub1));
  EXPECT_EQ(ProgressStateInterface::IDLE, stub1->state);

  ProgressStateInterfaceStub::Ptr const stub2 = ProgressStateInterfaceStub::create();
  EXPECT_TRUE(stub2);
  Stuff const r2 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub2));
  EXPECT_EQ(ProgressStateInterface::IDLE, stub2->state);

  progressBroadcaster->setWorking(0.0);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub1->state);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub2->state);
  EXPECT_EQ(0.0, stub1->progress);
  EXPECT_EQ(0.0, stub2->progress);

  progressBroadcaster->setWaiting(0.25);
  EXPECT_EQ(ProgressStateInterface::WAITING, stub1->state);
  EXPECT_EQ(ProgressStateInterface::WAITING, stub2->state);
  EXPECT_EQ(0.25, stub1->progress);
  EXPECT_EQ(0.25, stub2->progress);

  progressBroadcaster->setWorking(0.5);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub1->state);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub2->state);
  EXPECT_EQ(0.5, stub1->progress);
  EXPECT_EQ(0.5, stub2->progress);

  progressBroadcaster->setFinished();
  EXPECT_EQ(ProgressStateInterface::FINISHED, stub1->state);
  EXPECT_EQ(ProgressStateInterface::FINISHED, stub2->state);

  progressBroadcaster->setIdle();
  EXPECT_EQ(ProgressStateInterface::IDLE, stub1->state);
  EXPECT_EQ(ProgressStateInterface::IDLE, stub2->state);
}

TEST(ProgressInterfaceBroadcaster_Tests, late_children_receive_current_state) // NOLINT
{
  ProgressInterfaceBroadcaster::Ptr const progressBroadcaster = ProgressInterfaceBroadcaster::create();
  EXPECT_TRUE(progressBroadcaster);

  progressBroadcaster->setWorking(0.0);

  ProgressStateInterfaceStub::Ptr const stub1 = ProgressStateInterfaceStub::create();
  EXPECT_TRUE(stub1);
  Stuff const r1 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub1));
  EXPECT_EQ(ProgressStateInterface::WORKING, stub1->state);
  EXPECT_EQ(0.0, stub1->progress);

  progressBroadcaster->setWaiting(0.5);
  EXPECT_EQ(ProgressStateInterface::WAITING, stub1->state);
  EXPECT_EQ(0.5, stub1->progress);

  ProgressStateInterfaceStub::Ptr const stub2 = ProgressStateInterfaceStub::create();
  EXPECT_TRUE(stub2);
  Stuff const r2 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub2));

  EXPECT_EQ(ProgressStateInterface::WAITING, stub2->state);
  EXPECT_EQ(0.5, stub2->progress);
}

TEST(ProgressInterfaceBroadcaster_Tests, unsubscribed_children_stop_receiving_events) // NOLINT
{
  ProgressInterfaceBroadcaster::Ptr const progressBroadcaster = ProgressInterfaceBroadcaster::create();
  EXPECT_TRUE(progressBroadcaster);

  ProgressStateInterfaceStub::Ptr const stub1 = ProgressStateInterfaceStub::create();
  EXPECT_TRUE(stub1);
  Stuff r1 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub1));
  EXPECT_EQ(ProgressStateInterface::IDLE, stub1->state);

  ProgressStateInterfaceStub::Ptr const stub2 = ProgressStateInterfaceStub::create();
  EXPECT_TRUE(stub2);
  Stuff r2 = progressBroadcaster->subscribe(ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub2));
  EXPECT_EQ(ProgressStateInterface::IDLE, stub2->state);

  progressBroadcaster->setWorking(0.0);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub1->state);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub2->state);
  EXPECT_EQ(0.0, stub1->progress);
  EXPECT_EQ(0.0, stub2->progress);

  r1.reset();

  progressBroadcaster->setWaiting(0.25);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub1->state);
  EXPECT_EQ(0.0, stub1->progress);
  EXPECT_EQ(ProgressStateInterface::WAITING, stub2->state);
  EXPECT_EQ(0.25, stub2->progress);

  r2.reset();

  progressBroadcaster->setWorking(0.5);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub1->state);
  EXPECT_EQ(0.0, stub1->progress);
  EXPECT_EQ(ProgressStateInterface::WAITING, stub2->state);
  EXPECT_EQ(0.25, stub2->progress);
}
