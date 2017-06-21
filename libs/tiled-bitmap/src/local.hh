/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/threadpool.hh>
#include <boost/shared_ptr.hpp>

/** Prio for decompressing tiles */
#define LOAD_PRIO PRIO_HIGHER

/** Prio for fetching data from source medium */
#define DATAFETCH_PRIO PRIO_HIGH

/** Prio for reducing tiles that contain data */
#define REDUCE_PRIO PRIO_NORMAL

class MultithreadingData
{
public:
  typedef boost::shared_ptr<MultithreadingData const> ConstPtr;
  
  ThreadPool::Ptr cpuBound;
  PriorityRange priorityRange;
  ThreadPool::WeakQueue::Ptr queue;

public:
  static ConstPtr create(ThreadPool::Queue::Ptr queue);
  static ConstPtr create(ThreadPool::WeakQueue::Ptr queue);

  template<typename T>
  void scheduleHighPrio(T const& fn) const
  { cpuBound->schedule(fn, priorityRange.highest, queue); }

  template<typename T>
  void scheduleLowPrio(T const& fn) const
  { cpuBound->schedule(fn, priorityRange.lowest, queue); }

private:
  MultithreadingData(ThreadPool::WeakQueue::Ptr queue);
};
