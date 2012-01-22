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
/// ThreadPool::PrivateData
////////////////////////////////////////////////////////////////////////

ThreadPool::PrivateData::PrivateData(bool completeAllJobsBeforeDestruction)
  : jobcount(0), alive(true), completeAllJobsBeforeDestruction(completeAllJobsBeforeDestruction)
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
  count=1;
#endif
  add(count);
}

ThreadPool::ThreadPool(int count, bool completeAllJobsBeforeDestruction)
  : priv(PrivateData::create(completeAllJobsBeforeDestruction))
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
  ThreadPool::ThreadPtr t = ThreadPool::ThreadPtr(new boost::thread(boost::bind(&ThreadPool::work, priv)));
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
    boost::mutex::scoped_lock lock(priv->mut);
    priv->alive = false;
    priv->cond.notify_all();
  }

  while(!threads.empty())
  {
    ThreadPool::ThreadPtr t = threads.front();
    threads.pop_front();

    t->join();
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
      do_one(lock, priv);
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
      do_one(lock, priv);
    }
    else
    {
      busy = false;
    }
  }    
}

void ThreadPool::do_one(boost::mutex::scoped_lock& lock, ThreadPool::PrivateData::Ptr priv)
{
  while(!priv->jobs.empty() && priv->jobs.begin()->second.empty())
    priv->jobs.erase(priv->jobs.begin());

  // At this point, either the jobs map is empty, or jobs.begin()->second contains tasks.
  if(!priv->jobs.empty() && !priv->jobs.begin()->second.empty())
  {
    ThreadPool::Job job = priv->jobs.begin()->second.front();
    priv->jobs.begin()->second.pop();

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
