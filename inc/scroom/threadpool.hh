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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <queue>
#include <vector>

#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#ifndef NEW_BOOST_FUTURES
#  include <boost/future.hpp>
#endif

#include <scroom/semaphore.hh>

/** Priorities for scheduling on the ThreadPool */
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

/**
 * Generic threadpool
 *
 * A ThreadPool is basically a collection of threads you can
 * schedule() work on.
 */
class ThreadPool
{
public:
  typedef boost::shared_ptr<ThreadPool> Ptr;
  typedef boost::shared_ptr<boost::thread> ThreadPtr;
  
private:
  std::list<ThreadPtr> threads;        /**< Threads in this ThreadPool */
  Scroom::Semaphore jobcount;          /**< current number of tasks in ThreadPool::jobs */
  boost::mutex mut;                    /**< For protecting ThreadPool::jobs */

  /**
   * Jobs that remain to be executed
   *
   * The map contains for each priority that has jobs, a queue
   * containing those jobs. Once a queue is empty, it is removed from
   * the map. If a new job for that priority is scheduled, it is added
   * again.
   */
  std::map<int, std::queue<boost::function<void ()> > > jobs;

private:
  /**
   * This is executed by each of the threads in the ThreadPool
   *
   * @li Wait for a job to be scheduled
   * @li Fetch the highest-prio job from ThreadPool::jobs
   * @li Execute
   */
  void work();
  
public:
  /** Create a ThreadPool with one thread for each core in the system */
  ThreadPool();

  /** Create a ThreadPool with the given number of threads */
  ThreadPool(int count);

  /**
   * Destructor
   *
   * @li stop all threads
   * @li throw away all jobs that haven't been executed
   */
  ~ThreadPool();

  /** Schedule the given job at the given priority */
  void schedule(boost::function<void ()> const&& fn, int priority=PRIO_NORMAL);

  /**
   * Schedule the given job at the given priority
   *
   * @pre T::operator()() must be defined
   */
  template<typename T>
  void schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL);

#ifdef NEW_BOOST_FUTURES
#ifdef HAVE_STDCXX_0X
  template<typename R>
  boost::unique_future<R> schedule(boost::function<R ()> const& fn, int priority=PRIO_NORMAL);

  template<typename R, typename T>
  boost::unique_future<R> schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL);
#endif /* HAVE_STDCXX_0X */
#else /* NEW_BOOST_FUTURES */
  template<typename R>
  boost::future<R> schedule(boost::function<R ()> const& fn, int priority=PRIO_NORMAL);

  template<typename R, typename T>
  boost::future<R> schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL);
#endif /* NEW_BOOST_FUTURES */

  /**
   * Add an additional thread to the pool.
   *
   * This is mostly used for testing
   *
   * @return a reference to the newly added thread.
   */
  ThreadPtr add();

  /**
   * Add the given number of threads to the pool.
   *
   * This is mostly used for testing
   *
   * @return references to the newly added threads.
   */
  std::vector<ThreadPtr> add(int count);

};

/**
 * Jump the queue of a ThreadPool
 *
 * Have you ever seen a couple going shopping in the mall, where the
 * husband will go and wait in the queue for the register, while the
 * wife goes out into the shop and brings items to her husband,
 * already waiting to pay them? With QueueJumper, you'll get to be the
 * wife, while QueueJumper is the husband.
 *
 * You'll create() a QueueJumper, and schedule() it on a
 * ThreadPool. While the QueueJumper sits in the queue, waiting to be
 * processed, you can do whatever you want. At some later time, you'll
 * call setWork(). If the QueueJumper is still in the queue, it will
 * accept the work, and it will eventually get executed. Of course, if
 * the QueueJumper is no longer in the queue, you're out of luck.
 *
 * QueueJumper is used to maximally use the number of cores you have
 * available in your system, however many (or few) that is. Let's
 * assume you have a single threaded task (say, reading data from a
 * bitmap) that spawns a lot of background tasks (i.e. pre-scaling the
 * bitmap to some pre-defined zoom levels) that can be executed in
 * parallel. On any given system, two things can happen
 * @li The scaling is faster than the loading.
 * @li The scaling is slower than the loading.
 *
 * Let's assume for the moment that reading data from a bitmap is a
 * task that reads a given number of lines (say 1024 or some such),
 * and then reschedules itself. Let's assume also that all of these
 * threads are scheduled at the same priority. You'll start out as
 * follows:
@verbatim
| Queue | R
@endverbatim
 * Shown here is a ThreadPool queue, with one job in it, marked @c R,
 * because it is a Reading job. After the reading job has completed,
 * the queue will look as follows:
@verbatim
| Queue | S S S S S S S S S S 
@endverbatim
 * The queue now has some (say 10) scaling jobs in it, which can be
 * executed in parallel. Let's also reschedule our reading task:
@verbatim
| Queue | S S S S S S S S S S R
@endverbatim
 * Wait a minute! The scaling tasks can be executed in parallel, but
 * the reading task will be scheduled last, and then we'll be back to
 * waiting for it to complete, while my x-core machine is using only
 * one core. Let's fix that by executing our reading task at a higher
 * priority, such that it gets executed first.
@verbatim
| Higher prio | R
| Normal prio | S S S S S S S S S S
@endverbatim
 * In this scenario, our new reading task will be executed, while the
 * other cores are busy with the scaling jobs. When the scaling jobs
 * are finished, there are hopefully new scaling jobs waiting to be
 * processed.
 *
 * This will, in fact, work fine if the scaling is faster than the
 * reading. By the time the Reading task has scheduled new Scaling
 * jobs, all the previous ones will have finished. Problems may start,
 * however, if the Reading is faster than the Scaling. When the
 * Reading is finished (and has scheduled 10 new Scaling jobs), the
 * old scaling jobs still haven't completed. As a result, the number
 * of remaining scaling jobs will grow and grow. Worst case, on a
 * 1-core machine, no scaling job will get processed, because there is
 * always a reading job with a higher priority in the queue. That is,
 * of course, until all reading jobs have finished. Scaling will begin
 * only when the bitmap is completely loaded.
 *
 * For you, this may or may not be a problem. But if you are dealing
 * with very large bitmaps, then a scenario of first loading the
 * bitmap and then scaling it involves a lot of swapping: First you
 * load the bitmap, and as you load it, you'll swap out the older
 * unused parts. And then when you start scaling, you'll have to swap
 * those parts back in. It is much more efficient to do the scaling
 * before swapping out the old parts, and then swap out the old and
 * the scaled parts in one go. This is where QueueJumper can help out.
 *
 * Let's revisit for a moment the scenario where Reading jobs are
 * scheduled at a higher priority:
@verbatim
| Higher prio | R
| Normal prio | S S S S S S S S S S
@endverbatim
 * When the Reading job starts, the first thing it does is schedule a
 * QueueJumper:
@verbatim
| Higher prio | 
| Normal prio | S S S S S S S S S S Q
@endverbatim
 * Next, it will proceed to read bitmap data, which will result in
 * additional scaling jobs:
@verbatim
| Higher prio | 
| Normal prio | S S S S S S S S S S Q S S S S S S S S S S
@endverbatim
 * In the mean time, the other cores (if any) are busy with the
 * scaling jobs. Like we observed earlier, there are two possible
 * scenarios
 *
 * <b>Scaling is faster than reading</b><br>
 * When the reading job
 * completes, all earlier scaling jobs will have finished. Also, the
 * QueueJumper will have been processed (doing no work)
@verbatim
| Higher prio | 
| Normal prio | S S S S S S S S S S
@endverbatim
 * In this case, QueueJumper::setWork() will fail, because the
 * QueueJumper is no longer in the queue. You will have to reschedule
 * your reading task at the higher priority, like we did before:
@verbatim
| Higher prio | R
| Normal prio | S S S S S S S S S S
@endverbatim
 * In this scenario, you are not optimally using all cores, because
 * reading is a bottleneck.
 *
 * <b>Reading is faster than scaling</b><br>
 * When the reading job
 * completes, there will still be scaling jobs left in the queue. Also
 * the QueueJumper will still be there:
@verbatim
| Higher prio | 
| Normal prio | S S S S Q S S S S S S S S S S
@endverbatim
 * If we now schedule a Reading task at a higher priority, like before,
 * then the number of scaling tasks will grow and grow. But if we call
 * QueueJumper::setWork(), then we can effectively substitute the
 * QueueJumper with our reading task:
@verbatim
| Higher prio | 
| Normal prio | S S S S R S S S S S S S S S S
@endverbatim
 * This way, the reading task will not be started immediately, but
 * still sufficiently early. We can be confident that the new reading
 * task, too, is finished before the scaling tasks that succeed it.
 *
 * Memory behaviour is very attractive to: We need to have only two
 * "batches" in memory: One that is currently being read and one that
 * is currently being scaled. After those tasks complete, we'll not
 * need the data again until it is time to show the bitmap on the
 * screen.
 *
 * @see TiledBitmap
 */
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
  /**
   * Create a QueueJumper
   *
   * QueueJumper objects are managed as shared pointers, because there
   * will be two references to it:
   * @li one held by the ThreadPool, such that it can execute the job
   * @li one held by the original submitter, such that it can call setWork()
   *
   * The QueueJumper will be destroyed when both parties have released
   * their reference.
   */
  static Ptr create();

  /**
   * Set the function to be executed
   *
   * @param fn Function to be called from within operator()().
   *
   * @retval true if the QueueJumper is still in the queue (and the function will get executed)
   * @retval false if the QueueJumper no longer is in the queue (@c fn will be ignored)
   */
  bool setWork(boost::function<void ()> const& fn);

  void operator()();
};

/**
 * ThreadPool for cpu-bound tasks
 *
 * This ThreadPool has one thread per core in your system. For
 * CpuBound tasks, this is the optimum. Make sure your tasks do not
 * block or wait for anything.
 */
namespace CpuBound
{
  /**
   * Retrieve a shared pointer to this threadpool instance
   *
   * @see http://fuzzy.homedns.org/trac/scroom/wiki/StaticInitializationOrderFiasco
   */
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

  /**
   * Schedule jobs to be executed in parallel.
   *
   * Jobs with the highest priority will be scheduled first. However,
   * since there are multiple threads executing jobs, there is no
   * guarantee that no lower priority jobs will be started before the
   * last high-priority job is finished. You should do your own
   * synchronisation :-)
   */
  template<typename T>
  void schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL);
}

/**
 * ThreadPool for executing tasks sequentially
 *
 * Tasks that are cpu and/or memory intensive (think "loading large
 * bitmaps") are typically scheduled here, such that only one such
 * task runs at a time.
 *
 * Note that it is rare to do any actual work on this
 * thread. Especially the cpu-intensive tasks are best delegated to
 * CpuBound::schedule(). On this thread, you just wait for the work to
 * complete.
 */
namespace Sequentially
{
  /**
   * Retrieve a shared pointer to this threadpool instance
   *
   * @see http://fuzzy.homedns.org/trac/scroom/wiki/StaticInitializationOrderFiasco
   */
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

  /**
   * Schedule jobs to be executed in parallel.
   *
   * Jobs with the highest priority will be scheduled first. However,
   * since there are multiple threads executing jobs, there is no
   * guarantee that no lower priority jobs will be started before the
   * last high-priority job is finished. You should do your own
   * synchronisation :-)
   */
  template<typename T>
  void schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL);
}

#include <scroom/impl/threadpoolimpl.hh>

#endif
