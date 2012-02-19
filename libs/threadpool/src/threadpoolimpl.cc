/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include <scroom/threadpool.hh>
#include <scroom/async-deleter.hh>

#include "queue.hh"

using namespace Scroom::Detail::ThreadPool;

////////////////////////////////////////////////////////////////////////
/// ThreadPool
////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(bool completeAllJobsBeforeDestruction)
  : jobcount(0), alive(true), completeAllJobsBeforeDestruction(completeAllJobsBeforeDestruction)
{
  int count = boost::thread::hardware_concurrency();
#ifndef MULTITHREADING
  count=1;
#endif
  add(count);
}

ThreadPool::ThreadPool(int count, bool completeAllJobsBeforeDestruction)
  : jobcount(0), alive(true), completeAllJobsBeforeDestruction(completeAllJobsBeforeDestruction)
{
#ifndef MULTITHREADING
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
  ThreadPool::ThreadPtr t = ThreadPool::ThreadPtr(new boost::thread(boost::bind(&ThreadPool::work, this)));
  threads.push_back(t);
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
    boost::mutex::scoped_lock lock(mut);
    alive = false;
    cond.notify_all();
  }

  while(!threads.empty())
  {
    ThreadPool::ThreadPtr t = threads.front();
    threads.pop_front();

    t->join();
  }
}

void ThreadPool::work()
{
  boost::mutex::scoped_lock lock(mut);
  while(alive)
  {
    if(jobcount>0)
    {
      jobcount--;
      do_one(lock);
    }
    else
    {
      cond.wait(lock);
    }
  }

  bool busy = completeAllJobsBeforeDestruction;
  while(busy)
  {
    if(jobcount>0)
    {
      jobcount--;
      do_one(lock);
    }
    else
    {
      busy = false;
    }
  }    
}

void ThreadPool::do_one(boost::mutex::scoped_lock& lock)
{
  while(!jobs.empty() && jobs.begin()->second.empty())
    jobs.erase(jobs.begin());

  // At this point, either the jobs map is empty, or jobs.begin()->second contains tasks.
  if(!jobs.empty() && !jobs.begin()->second.empty())
  {
    ThreadPool::Job job = jobs.begin()->second.front();
    jobs.begin()->second.pop();

    QueueLock l(job.queue);
    if(l.queueExists())
    {
      lock.unlock();

      try
      {
        boost::this_thread::disable_interruption while_executing_jobs;
        job.fn();
      }
      catch(...)
      {
        lock.lock();
        throw;
      }
      lock.lock();
    }
  }
  else
  {
    printf("PANIC: JobQueue empty while it shouldn't be\n");
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
  boost::mutex::scoped_lock lock(mut);
  jobs[priority].push(Job(fn, queue));
  jobcount++;
  cond.notify_one();
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

ThreadPool::Job::Job(boost::function<void ()> fn, WeakQueue::Ptr queue)
:queue(queue->get()), fn(fn)
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

bool QueueJumper::setWork(boost::function<void ()> const& fn)
{
  boost::mutex::scoped_lock lock(mut);
  if(inQueue)
  {
    // Our turn hasn't passed yet. Accept work.
    this->fn = fn;
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
