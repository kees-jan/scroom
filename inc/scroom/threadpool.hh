/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

#ifndef _THREADPOOL_HH
#define _THREADPOOL_HH

#include <queue>

#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <scroom/semaphore.hh>

enum
  {
    PRIO_HIGHEST = 100,
    PRIO_HIGHER,
    PRIO_HIGH,
    PRIO_NORMAL,
    PRIO_LOW,
    PRIO_LOWER,
    PRIO_LOWEST,
  };

class ThreadPool
{
public:
  typedef boost::shared_ptr<ThreadPool> Ptr;
  
private:
  std::list<boost::thread*> threads;
  Scroom::Semaphore jobcount;
  std::map<int, std::queue<boost::function<void ()> > > jobs;
  boost::mutex mut;
  boost::mutex threadsMut;

private:
  void work();
  
public:
  ThreadPool();
  ThreadPool(int count);
  ~ThreadPool();
  void schedule(boost::function<void ()> const& fn, int priority=PRIO_NORMAL);

  template<typename T>
  void schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL);
};

class QueueJumper
{
public:
  typedef boost::shared_ptr<QueueJumper> Ptr;
  
private:
  boost::mutex mut;
  bool inQueue;
  bool isSet;

  boost::function<void ()> fn;

protected:
  QueueJumper();
  
public:
  static Ptr create();

  bool setWork(boost::function<void ()> const& fn);
  bool isDone();

  void operator()();
};

namespace CpuBound
{
  ThreadPool::Ptr instance();
  
  /**
   * Schedule jobs to be executed in parallel.
   *
   * Jobs with the highest priority will be scheduled first. However,
   * since there are multiple threads executing jobs, there is no
   * guarantee that no lower priority jobs will be started before the
   * last high-priority job is finished. You should do your own
   * synchronisation :-)
   */
  void schedule(boost::function<void ()> const& fn, int priority=PRIO_NORMAL);

  template<typename T>
  void schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL);
}

namespace Sequentially
{
  ThreadPool::Ptr instance();
  
  /**
   * Schedule jobs to be executed in parallel.
   *
   * Jobs with the highest priority will be scheduled first. However,
   * since there are multiple threads executing jobs, there is no
   * guarantee that no lower priority jobs will be started before the
   * last high-priority job is finished. You should do your own
   * synchronisation :-)
   */
  void schedule(boost::function<void ()> const& fn, int priority=PRIO_NORMAL);

  template<typename T>
  void schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL);
}

////////////////////////////////////////////////////////////////////////
// Code...

namespace
{
  template<typename T>
  void threadPoolExecute(boost::shared_ptr<T> fn)
  {
    (*fn)();
  }
}

template<typename T>
void ThreadPool::schedule(boost::shared_ptr<T> fn, int priority)
{
  schedule(priority, boost::bind(threadPoolExecute, fn));
}

namespace CpuBound
{
  template<typename T>
  void schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL)
  {
    schedule(boost::bind(threadPoolExecute<T>, fn), priority);
  }
}

namespace Sequentially
{
  template<typename T>
  void schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL)
  {
    schedule(boost::bind(threadPoolExecute<T>, fn), priority);
  }
}

#endif
