/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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

////////////////////////////////////////////////////////////////////////
/// ThreadPool
////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool()
{
  int count = boost::thread::hardware_concurrency();
#ifndef MULTITHREADING
  count=1;
#endif
  add(count);
}

ThreadPool::ThreadPool(int count)
{
#ifndef MULTITHREADING
  count=1;
#endif
  add(count);
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
  // printf("Attempting to destroy the threadpool...\n");

  while(!threads.empty())
  {
    ThreadPool::ThreadPtr t = threads.front();
    threads.pop_front();
    
    t->interrupt();
    t->join();
  }
  // printf("Done destroying threadpool\n");
}

void ThreadPool::work()
{
  while(true)
  {
    jobcount.P();
    boost::unique_lock<boost::mutex> lock(mut);

    while(!jobs.empty() && jobs.begin()->second.empty())
      jobs.erase(jobs.begin());

    // At this point, either the jobs map is empty, or jobs.begin()->second contains tasks.
  
    if(!jobs.empty() && !jobs.begin()->second.empty())
    {
      ThreadPool::Job job = jobs.begin()->second.front();
      jobs.begin()->second.pop();

      ThreadPool::Queue::Ptr queue = job.queue.lock();
      if(queue)
      {
        ThreadPool::QueueLock l(queue);

        // Release our reference to the queue, before one of our other
        // threads incorrectly assumes the queue still exists
        queue.reset();
        lock.unlock();


        boost::this_thread::disable_interruption while_executing_jobs;
        job.fn();
      }
    }
    else
    {
      printf("PANIC: JobQueue empty while it shouldn't be\n");
    }
  }
}

void ThreadPool::schedule(boost::function<void ()> const& fn, int priority, ThreadPool::Queue::Ptr queue)
{
  boost::unique_lock<boost::mutex> lock(mut);
  jobs[priority].push(Job(fn, queue));
  jobcount.V();
}

void ThreadPool::schedule(boost::function<void ()> const& fn, ThreadPool::Queue::Ptr queue)
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

ThreadPool::Queue::Queue()
: mut(), cond(), count(0)
{
}

ThreadPool::Queue::~Queue()
{
  boost::mutex::scoped_lock lock(mut);
  while(count!=0)
  {
    cond.wait(lock);
  }
}

void ThreadPool::Queue::jobStarted()
{
  boost::mutex::scoped_lock lock(mut);
  count++;
}

void ThreadPool::Queue::jobFinished()
{
  boost::mutex::scoped_lock lock(mut);
  count--;
  cond.notify_all();
}

////////////////////////////////////////////////////////////////////////
/// ThreadPool::QueueLock
////////////////////////////////////////////////////////////////////////

ThreadPool::QueueLock::QueueLock(Queue::Ptr queue)
:q(queue.get())
{
  q->jobStarted();
}

ThreadPool::QueueLock::~QueueLock()
{
  q->jobFinished();
}

////////////////////////////////////////////////////////////////////////
/// ThreadPool::Job
////////////////////////////////////////////////////////////////////////

ThreadPool::Job::Job()
: queue(), fn()
{
}

ThreadPool::Job::Job(boost::function<void ()> fn, Queue::WeakPtr queue)
:queue(queue), fn(fn)
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
  boost::unique_lock<boost::mutex> lock(mut);
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
  boost::unique_lock<boost::mutex> lock(mut);
  if(isSet)
  {
    fn();
  }
  inQueue=false;
}
