/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include <scroom/assertions.hh>
#include <scroom/async-deleter.hh>
#include <scroom/threadpool.hh>

#include "queue.hh"

using namespace Scroom::Detail::ThreadPool;

////////////////////////////////////////////////////////////////////////
/// ThreadList / ThreadWaiter
////////////////////////////////////////////////////////////////////////

namespace
{
  /**
   * List of threads to wait for (when terminating)
   */
  class ThreadList
  {
  public:
    using Ptr = std::shared_ptr<ThreadList>;

  private:
    boost::mutex                                           mut;
    std::list<ThreadPool::ThreadPtr>                       threads;
    std::list<std::pair<std::weak_ptr<void>, std::string>> pointers;

  private:
    void dumpPointers();

  public:
    static Ptr instance();
    void       wait();
    void       add(const ThreadPool::ThreadPtr& t);
    void       add(const std::shared_ptr<void>& t, const std::string& s);
  };

  /**
   * Actually wait for all threads registered to the ThreadList
   *
   * Waiting is done on the main thread.
   */
  class ThreadWaiter
  {
  private:
    ThreadList::Ptr threadList;

  public:
    ThreadWaiter();
    ~ThreadWaiter();
    ThreadWaiter(const ThreadWaiter&)           = delete;
    ThreadWaiter(ThreadWaiter&&)                = delete;
    ThreadWaiter operator=(const ThreadWaiter&) = delete;
    ThreadWaiter operator=(ThreadWaiter&&)      = delete;
  };

  ThreadWaiter waiter;

  template <typename T>
  std::shared_ptr<T> NotifyThreadList(std::shared_ptr<T> t, const std::string& s)
  {
    ThreadList::instance()->add(t, s);
    return t;
  }

  ////////////////////////////////////////////////////////////////////////

  ThreadList::Ptr ThreadList::instance()
  {
    static Ptr const threadList = std::make_shared<ThreadList>();
    return threadList;
  }

  void ThreadList::dumpPointers()
  {
    auto cur = pointers.begin();
    while(cur != pointers.end())
    {
      const auto count = cur->first.use_count();

      if(count)
      {
        spdlog::info("{} references to {} remaining", count, cur->second);
        cur++;
      }
      else
      {
        cur = pointers.erase(cur);
      }
    }
  }

  void ThreadList::wait()
  {
    const boost::posix_time::millisec short_timeout(1);
    const boost::posix_time::millisec timeout(250);
    decltype(threads.size())          count = 0;

    {
      boost::mutex::scoped_lock const lock(mut);

      auto cur = threads.begin();
      while(cur != threads.end())
      {
        if((*cur)->timed_join(short_timeout))
        {
          cur = threads.erase(cur);
        }
        else
        {
          cur++;
        }
      }

      count = threads.size();
    }

    int triesRemaining = 256;
    while(triesRemaining > 0 && count > 0)
    {
      boost::mutex::scoped_lock const lock(mut);
      dumpPointers();
      spdlog::info("Waiting for {} threads to terminate", count);

      auto cur = threads.begin();
      while(cur != threads.end())
      {
        if((*cur)->timed_join(timeout))
        {
          cur = threads.erase(cur);
        }
        else
        {
          cur++;
        }
      }

      count = threads.size();
      triesRemaining--;
    }

    if(!threads.empty())
    {
      abort();
    }
  }

  void ThreadList::add(const ThreadPool::ThreadPtr& t)
  {
    boost::mutex::scoped_lock const lock(mut);
    threads.push_back(t);
  }

  void ThreadList::add(const std::shared_ptr<void>& t, const std::string& s)
  {
    boost::mutex::scoped_lock const lock(mut);
    pointers.push_back(std::make_pair<std::weak_ptr<void>, std::string>(t, std::string(s)));
  }

  ThreadWaiter::ThreadWaiter()
    : threadList(ThreadList::instance())
  {
  }

  ThreadWaiter::~ThreadWaiter() { threadList->wait(); }
} // namespace

////////////////////////////////////////////////////////////////////////
/// ThreadPool::PrivateData
////////////////////////////////////////////////////////////////////////

ThreadPool::PrivateData::PrivateData(bool completeAllJobsBeforeDestruction_)
  : completeAllJobsBeforeDestruction(completeAllJobsBeforeDestruction_)
  , defaultQueue(ThreadPool::defaultQueue())
{
}

ThreadPool::PrivateData::Ptr ThreadPool::PrivateData::create(bool completeAllJobsBeforeDestruction)
{
  return Ptr(new PrivateData(completeAllJobsBeforeDestruction));
}

////////////////////////////////////////////////////////////////////////
/// ThreadPool
////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(bool completeAllJobsBeforeDestruction)
  : priv(PrivateData::create(completeAllJobsBeforeDestruction))
{
  const int count = boost::thread::hardware_concurrency();
#ifndef MULTITHREADING
  if(count > 1)
    count = 1;
#endif
  add(count);
}

ThreadPool::ThreadPool(int count, bool completeAllJobsBeforeDestruction)
  : priv(PrivateData::create(completeAllJobsBeforeDestruction))
{
#ifndef MULTITHREADING
  if(count > 1)
    count = 1;
#endif
  add(count);
}

ThreadPool::Ptr ThreadPool::create(bool completeAllJobsBeforeDestruction)
{
  return std::make_shared<ThreadPool>(completeAllJobsBeforeDestruction);
}

ThreadPool::Ptr ThreadPool::create(int count, bool completeAllJobsBeforeDestruction)
{
  return std::make_shared<ThreadPool>(count, completeAllJobsBeforeDestruction);
}

ThreadPool::ThreadPtr ThreadPool::add()
{
  auto t = std::make_shared<boost::thread>([data = priv] { ThreadPool::work(data); });
  threads.push_back(t);
  ThreadList::instance()->add(t);
  return t;
}

std::vector<ThreadPool::ThreadPtr> ThreadPool::add(int count)
{
  std::vector<ThreadPool::ThreadPtr> result(count);
  for(int i = 0; i < count; i++)
  {
    result[i] = add();
  }

  return result;
}

ThreadPool::~ThreadPool()
{
  // Destroying the threadpool used to be done by interrupting all
  // threads, but this doesn't work reliably, at least until boost
  // 1.45. Hence, we're back to using an "alive" boolean and a regular
  // condition variable.
  //
  // See also https://svn.boost.org/trac/boost/ticket/2330

  {
    boost::mutex::scoped_lock const lock(priv->mut);
    priv->alive = false;
    priv->cond.notify_all();
  }
}

void ThreadPool::work(const ThreadPool::PrivateData::Ptr& priv)
{
  boost::mutex::scoped_lock lock(priv->mut);
  while(priv->alive)
  {
    if(priv->jobcount > 0)
    {
      priv->jobcount--;
      lock.unlock();
      do_one(priv);
      lock.lock();
    }
    else
    {
      priv->cond.wait(lock);
    }
  }

  bool busy = priv->completeAllJobsBeforeDestruction;
  while(busy)
  {
    if(priv->jobcount > 0)
    {
      priv->jobcount--;
      lock.unlock();
      do_one(priv);
      lock.lock();
    }
    else
    {
      busy = false;
    }
  }
}

void ThreadPool::do_one(const ThreadPool::PrivateData::Ptr& priv)
{
  ThreadPool::Job job;

  {
    boost::mutex::scoped_lock const lock(priv->mut);

    while(!priv->jobs.empty() && priv->jobs.begin()->second.empty())
    {
      priv->jobs.erase(priv->jobs.begin());
    }

    if(!priv->jobs.empty() && !priv->jobs.begin()->second.empty())
    {
      job = priv->jobs.begin()->second.front();
      priv->jobs.begin()->second.pop();
    }
    else
    {
      defect_message("JobQueue empty while it shouldn't be");
    }
  }

  if(job.queue)
  {
    QueueLock const l(job.queue);
    if(l.queueExists())
    {
      boost::this_thread::disable_interruption const while_executing_jobs;
      job.fn();
    }
  }
}

void ThreadPool::schedule(boost::function<void()> const& fn, int priority, const ThreadPool::Queue::Ptr& queue)
{
  schedule(fn, priority, queue->getWeak());
}

void ThreadPool::schedule(boost::function<void()> const& fn, const ThreadPool::Queue::Ptr& queue)
{
  schedule(fn, defaultPriority, std::move(queue));
}

void ThreadPool::schedule(boost::function<void()> const& fn, int priority, const ThreadPool::WeakQueue::Ptr& queue)
{
  boost::mutex::scoped_lock const lock(priv->mut);
  priv->jobs[priority].emplace(fn, queue);
  priv->jobcount++;
  priv->cond.notify_one();
}

void ThreadPool::schedule(boost::function<void()> const& fn, const ThreadPool::WeakQueue::Ptr& queue)
{
  schedule(fn, defaultPriority, std::move(queue));
}

ThreadPool::Queue::Ptr ThreadPool::defaultQueue()
{
  static ThreadPool::Queue::Ptr const queue = ThreadPool::Queue::create();
  return queue;
}

const int ThreadPool::defaultPriority = PRIO_NORMAL;

////////////////////////////////////////////////////////////////////////
/// ThreadPool::Queue
////////////////////////////////////////////////////////////////////////

ThreadPool::Queue::Ptr ThreadPool::Queue::create() { return ThreadPool::Queue::Ptr(new ThreadPool::Queue()); }

ThreadPool::Queue::Ptr ThreadPool::Queue::createAsync() { return {new ThreadPool::Queue(), AsyncDeleter<ThreadPool::Queue>()}; }

ThreadPool::Queue::Queue()
  : weak(WeakQueue::create())
{
}

ThreadPool::Queue::~Queue() { weak->get()->deletingQueue(); }

QueueImpl::Ptr ThreadPool::Queue::get() { return weak->get(); }

ThreadPool::WeakQueue::Ptr ThreadPool::Queue::getWeak() { return weak; }

////////////////////////////////////////////////////////////////////////
/// ThreadPool::WeakQueue
////////////////////////////////////////////////////////////////////////

ThreadPool::WeakQueue::Ptr ThreadPool::WeakQueue::create() { return ThreadPool::WeakQueue::Ptr(new ThreadPool::WeakQueue()); }

ThreadPool::WeakQueue::WeakQueue()
  : qi(QueueImpl::create())
{
}

QueueImpl::Ptr ThreadPool::WeakQueue::get() { return qi; }

////////////////////////////////////////////////////////////////////////
/// ThreadPool::Job
////////////////////////////////////////////////////////////////////////

ThreadPool::Job::Job(boost::function<void()> fn_, const WeakQueue::Ptr& queue_)
  : queue(queue_->get())
  , fn(std::move(fn_))
{
}

////////////////////////////////////////////////////////////////////////
/// QueueJumper
////////////////////////////////////////////////////////////////////////

QueueJumper::Ptr QueueJumper::create() { return QueueJumper::Ptr(new QueueJumper()); }

bool QueueJumper::setWork(boost::function<void()> const& fn_)
{
  boost::mutex::scoped_lock const lock(mut);
  if(inQueue)
  {
    // Our turn hasn't passed yet. Accept work.
    fn    = fn_;
    isSet = true;
  }
  else
  {
    // Our turn has passed. We cannot do the work
  }

  return inQueue;
}

void QueueJumper::operator()()
{
  boost::mutex::scoped_lock const lock(mut);
  if(isSet)
  {
    fn();
  }
  inQueue = false;
}

////////////////////////////////////////////////////////////////////////
// Default threadpools
////////////////////////////////////////////////////////////////////////

ThreadPool::Ptr CpuBound()
{
  static ThreadPool::Ptr const cpuBound = NotifyThreadList<ThreadPool>(std::make_shared<ThreadPool>(), "CpuBound threadpool");

  return cpuBound;
}

ThreadPool::Ptr Sequentially()
{
  static ThreadPool::Ptr const sequentially =
    NotifyThreadList<ThreadPool>(std::make_shared<ThreadPool>(1), "Sequential threadpool");

  return sequentially;
}
