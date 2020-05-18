/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include <utility>

#include <scroom/threadpool.hh>
#include <scroom/async-deleter.hh>

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
    typedef boost::shared_ptr<ThreadList> Ptr;

  private:
    boost::mutex mut;
    std::list<ThreadPool::ThreadPtr> threads;
    std::list<std::pair<boost::weak_ptr<void>, std::string> > pointers;

  private:
    void dumpPointers();

  public:
    static Ptr instance();
    void wait();
    void add(ThreadPool::ThreadPtr t);
    void add(boost::shared_ptr<void> t, const std::string& s);
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
  };

  ThreadWaiter waiter;

  template<typename T>
  class NotifyThreadList
  {
  private:
    boost::shared_ptr<T> t;

  public:
    NotifyThreadList(boost::shared_ptr<T> t_, const std::string& s) : t(t_) { ThreadList::instance()->add(t_, s); }

    operator boost::shared_ptr<T> () { return t; }
  };

  ////////////////////////////////////////////////////////////////////////

  ThreadList::Ptr ThreadList::instance()
  {
    static Ptr threadList = Ptr(new ThreadList());
    return threadList;
  }

  void ThreadList::dumpPointers()
  {
    std::list<std::pair<boost::weak_ptr<void>, std::string> >::iterator cur = pointers.begin();
    while(cur != pointers.end())
    {
      const int count = cur->first.use_count();

      if(count)
      {
        printf("%d references to %s remaining\n", count, cur->second.c_str());
        cur++;
      }
      else
        cur=pointers.erase(cur);
    }
  }

  void ThreadList::wait()
  {
    const boost::posix_time::millisec short_timeout(1);
    const boost::posix_time::millisec timeout(250);
    int count=0;

    {
      boost::mutex::scoped_lock lock(mut);

      std::list<ThreadPool::ThreadPtr>::iterator cur = threads.begin();
      while(cur != threads.end())
      {
        if((*cur)->timed_join(short_timeout))
          cur=threads.erase(cur);
        else
          cur++;
      }

      count=threads.size();
    }

    int triesRemaining = 256;
    printf("\n");
    while(triesRemaining>0 && count>0)
    {
      boost::mutex::scoped_lock lock(mut);
      dumpPointers();
      printf("Waiting for %d threads to terminate", static_cast<unsigned int>(count));

      std::list<ThreadPool::ThreadPtr>::iterator cur = threads.begin();
      while(cur != threads.end())
      {
        if((*cur)->timed_join(timeout))
          cur=threads.erase(cur);
        else
          cur++;

        printf(".");
      }

      printf("\n");
      count=threads.size();
      triesRemaining--;
    }

    if(0<threads.size())
      abort();
  }

  void ThreadList::add(ThreadPool::ThreadPtr t)
  {
    boost::mutex::scoped_lock lock(mut);
    threads.push_back(t);
  }

  void ThreadList::add(boost::shared_ptr<void> t, const std::string& s)
  {
    boost::mutex::scoped_lock lock(mut);
    pointers.push_back(std::make_pair<boost::weak_ptr<void>,std::string>(t, std::string(s)));
  }

  ThreadWaiter::ThreadWaiter()
    : threadList(ThreadList::instance())
  {}

  ThreadWaiter::~ThreadWaiter()
  {
    threadList->wait();
  }
}

////////////////////////////////////////////////////////////////////////
/// ThreadPool::PrivateData
////////////////////////////////////////////////////////////////////////

ThreadPool::PrivateData::PrivateData(bool completeAllJobsBeforeDestruction_)
  : jobcount(0), alive(true), completeAllJobsBeforeDestruction(completeAllJobsBeforeDestruction_), defaultQueue(ThreadPool::defaultQueue())
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
  int count = boost::thread::hardware_concurrency();
#ifndef MULTITHREADING
  if(count>1)
    count=1;
#endif
  add(count);
}

ThreadPool::ThreadPool(int count, bool completeAllJobsBeforeDestruction)
  : priv(PrivateData::create(completeAllJobsBeforeDestruction))
{
#ifndef MULTITHREADING
  if(count>1)
    count=1;
#endif
  add(count);
}

ThreadPool::Ptr ThreadPool::create(bool completeAllJobsBeforeDestruction)
{
  return ThreadPool::Ptr(new ThreadPool(completeAllJobsBeforeDestruction));
}

ThreadPool::Ptr ThreadPool::create(int count, bool completeAllJobsBeforeDestruction)
{
  return ThreadPool::Ptr(new ThreadPool(count, completeAllJobsBeforeDestruction));
}

ThreadPool::ThreadPtr ThreadPool::add()
{
  ThreadPool::ThreadPtr t = ThreadPool::ThreadPtr(new boost::thread(boost::bind(&ThreadPool::work, priv)));
  threads.push_back(t);
  ThreadList::instance()->add(t);
  return t;
}

std::vector<ThreadPool::ThreadPtr> ThreadPool::add(int count)
{
  std::vector<ThreadPool::ThreadPtr> result(count);
  for(int i=0; i<count; i++)
    result[i] = add();

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
    boost::mutex::scoped_lock lock(priv->mut);
    priv->alive = false;
    priv->cond.notify_all();
  }
}

void ThreadPool::work(ThreadPool::PrivateData::Ptr priv)
{
  boost::mutex::scoped_lock lock(priv->mut);
  while(priv->alive)
  {
    if(priv->jobcount>0)
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
    if(priv->jobcount>0)
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

void ThreadPool::do_one(ThreadPool::PrivateData::Ptr priv)
{
  ThreadPool::Job job;

  {
    boost::mutex::scoped_lock lock(priv->mut);

    while(!priv->jobs.empty() && priv->jobs.begin()->second.empty())
      priv->jobs.erase(priv->jobs.begin());

    if(!priv->jobs.empty() && !priv->jobs.begin()->second.empty())
    {
      job = priv->jobs.begin()->second.front();
      priv->jobs.begin()->second.pop();
    }
    else
    {
      printf("PANIC: JobQueue empty while it shouldn't be\n");
    }
  }

  if(job.queue)
  {
    QueueLock l(job.queue);
    if(l.queueExists())
    {
      boost::this_thread::disable_interruption while_executing_jobs;
      job.fn();
    }
  }
}

void ThreadPool::schedule(boost::function<void ()> const& fn, int priority, ThreadPool::Queue::Ptr queue)
{
  schedule(fn, priority, queue->getWeak());
}

void ThreadPool::schedule(boost::function<void ()> const& fn, ThreadPool::Queue::Ptr queue)
{
  schedule(fn, defaultPriority, queue);
}

void ThreadPool::schedule(boost::function<void ()> const& fn, int priority, ThreadPool::WeakQueue::Ptr queue)
{
  boost::mutex::scoped_lock lock(priv->mut);
  priv->jobs[priority].push(Job(fn, queue));
  priv->jobcount++;
  priv->cond.notify_one();
}

void ThreadPool::schedule(boost::function<void ()> const& fn, ThreadPool::WeakQueue::Ptr queue)
{
  schedule(fn, defaultPriority, queue);
}

ThreadPool::Queue::Ptr ThreadPool::defaultQueue()
{
  static ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  return queue;
}

const int ThreadPool::defaultPriority = PRIO_NORMAL;

////////////////////////////////////////////////////////////////////////
/// ThreadPool::Queue
////////////////////////////////////////////////////////////////////////

ThreadPool::Queue::Ptr ThreadPool::Queue::create()
{
  return ThreadPool::Queue::Ptr(new ThreadPool::Queue());
}

ThreadPool::Queue::Ptr ThreadPool::Queue::createAsync()
{
  return ThreadPool::Queue::Ptr(new ThreadPool::Queue(), AsyncDeleter<ThreadPool::Queue>());
}

ThreadPool::Queue::Queue()
: weak(WeakQueue::create())
{
}

ThreadPool::Queue::~Queue()
{
  weak->get()->deletingQueue();
}

QueueImpl::Ptr ThreadPool::Queue::get()
{
  return weak->get();
}

ThreadPool::WeakQueue::Ptr ThreadPool::Queue::getWeak()
{
  return weak;
}

////////////////////////////////////////////////////////////////////////
/// ThreadPool::WeakQueue
////////////////////////////////////////////////////////////////////////

ThreadPool::WeakQueue::Ptr ThreadPool::WeakQueue::create()
{
  return ThreadPool::WeakQueue::Ptr(new ThreadPool::WeakQueue());
}

ThreadPool::WeakQueue::WeakQueue()
: qi(QueueImpl::create())
{
}

ThreadPool::WeakQueue::~WeakQueue()
{
}

QueueImpl::Ptr ThreadPool::WeakQueue::get()
{
  return qi;
}

////////////////////////////////////////////////////////////////////////
/// ThreadPool::Job
////////////////////////////////////////////////////////////////////////

ThreadPool::Job::Job()
: queue(), fn()
{
}

ThreadPool::Job::Job(boost::function<void ()> fn_, WeakQueue::Ptr queue_)
:queue(queue_->get()), fn(fn_)
{
}

////////////////////////////////////////////////////////////////////////
/// QueueJumper
////////////////////////////////////////////////////////////////////////

QueueJumper::QueueJumper()
  : mut(), inQueue(true), isSet(false)
{}

QueueJumper::Ptr QueueJumper::create()
{
  return QueueJumper::Ptr(new QueueJumper());
}

bool QueueJumper::setWork(boost::function<void ()> const& fn_)
{
  boost::mutex::scoped_lock lock(mut);
  if(inQueue)
  {
    // Our turn hasn't passed yet. Accept work.
    this->fn = fn_;
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
  boost::mutex::scoped_lock lock(mut);
  if(isSet)
  {
    fn();
  }
  inQueue=false;
}

////////////////////////////////////////////////////////////////////////
// Default threadpools
////////////////////////////////////////////////////////////////////////

ThreadPool::Ptr CpuBound()
{
  static ThreadPool::Ptr cpuBound = NotifyThreadList<ThreadPool>(ThreadPool::Ptr(new ThreadPool()), "CpuBound threadpool");

  return cpuBound;
}

ThreadPool::Ptr Sequentially()
{
  static ThreadPool::Ptr sequentially = NotifyThreadList<ThreadPool>(ThreadPool::Ptr(new ThreadPool(1)), "Sequential threadpool");

  return sequentially;
}

