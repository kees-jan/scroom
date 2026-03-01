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

TEST(test_converters, test_ProgressInterfaceFromProgressStateInterface) // NOLINT
{
  ProgressStateInterfaceStub::Ptr const stub = ProgressStateInterfaceStub::create();
  ASSERT_TRUE(stub);
  EXPECT_EQ(ProgressStateInterface::IDLE, stub->state);

  ProgressInterface::Ptr const pi = ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub);
  ASSERT_TRUE(pi);
  EXPECT_EQ(ProgressStateInterface::IDLE, stub->state);

  pi->setWaiting();
  EXPECT_EQ(ProgressStateInterface::WAITING, stub->state);
  EXPECT_EQ(0.0, stub->progress);

  pi->setWorking(0.33);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.33, stub->progress);

  pi->setWaiting(0.25);
  EXPECT_EQ(ProgressStateInterface::WAITING, stub->state);
  EXPECT_EQ(0.25, stub->progress);

  pi->setFinished();
  EXPECT_EQ(ProgressStateInterface::FINISHED, stub->state);
  EXPECT_EQ(1.0, stub->progress);

  pi->setWorking(0.75);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.75, stub->progress);

  pi->setIdle();
  EXPECT_EQ(ProgressStateInterface::IDLE, stub->state);
}

TEST(test_converters, test_ProgressStateInterfaceFromProgressInterface) // NOLINT
{
  ProgressStateInterfaceStub::Ptr const stub = ProgressStateInterfaceStub::create();
  ASSERT_TRUE(stub);
  EXPECT_EQ(ProgressStateInterface::IDLE, stub->state);

  ProgressInterface::Ptr const pi = ProgressInterfaceFromProgressStateInterfaceForwarder::create(stub);
  ASSERT_TRUE(pi);
  EXPECT_EQ(ProgressStateInterface::IDLE, stub->state);

  ProgressStateInterface::Ptr const ps = ProgressStateInterfaceFromProgressInterfaceForwarder::create(pi);
  ASSERT_TRUE(ps);
  EXPECT_EQ(ProgressStateInterface::IDLE, stub->state);

  stub->progress = 0.33;

  ps->setProgress(ProgressStateInterface::WAITING);
  EXPECT_EQ(ProgressStateInterface::WAITING, stub->state);
  EXPECT_EQ(0.0, stub->progress);

  ps->setProgress(ProgressStateInterface::WORKING, 0.27);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.27, stub->progress);

  ps->setProgress(ProgressStateInterface::WAITING, 0.27);
  EXPECT_EQ(ProgressStateInterface::WAITING, stub->state);
  EXPECT_EQ(0.27, stub->progress);

  ps->setProgress(ProgressStateInterface::FINISHED);
  EXPECT_EQ(ProgressStateInterface::FINISHED, stub->state);
  EXPECT_EQ(1.0, stub->progress);

  ps->setProgress(ProgressStateInterface::WORKING, 0.75);
  EXPECT_EQ(ProgressStateInterface::WORKING, stub->state);
  EXPECT_EQ(0.75, stub->progress);

  ps->setProgress(ProgressStateInterface::IDLE);
  EXPECT_EQ(ProgressStateInterface::IDLE, stub->state);
}
