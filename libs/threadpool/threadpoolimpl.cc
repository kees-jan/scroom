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
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include <scroom/threadpool.hh>

////////////////////////////////////////////////////////////////////////
/// NoWork
////////////////////////////////////////////////////////////////////////

class NoWork : public WorkInterface
{
public:
  virtual bool doWork();
};

bool NoWork::doWork()
{
  return false;
}

////////////////////////////////////////////////////////////////////////
/// BoostFunctionWork
////////////////////////////////////////////////////////////////////////

class BoostFunctionWork : public WorkInterface
{
private:
  boost::function<void ()> const fn;
  
public:
  BoostFunctionWork(boost::function<void ()> const& fn);
  virtual ~BoostFunctionWork();
  virtual bool doWork();
};

BoostFunctionWork::BoostFunctionWork(boost::function<void ()> const& fn)
  : fn(fn)
{
}

BoostFunctionWork::~BoostFunctionWork()
{
}

bool BoostFunctionWork::doWork()
{
  fn();
  return false;
}

////////////////////////////////////////////////////////////////////////
/// ThreadPool
////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool()
{
  int count = boost::thread::hardware_concurrency();
#ifndef MULTITHREADING
  count=1;
#endif
  alive=true;
  printf("Starting ThreadPool with %d threads\n", count);
  for(int i=0; i<count; i++)
  {
    threads.push_back(new boost::thread(boost::bind(&ThreadPool::work, this)));
  }
}

ThreadPool::ThreadPool(int count)
{
#ifndef MULTITHREADING
  count=1;
#endif
  alive=true;
  printf("Starting ThreadPool with %d threads\n", count);
  for(int i=0; i<count; i++)
  {
    threads.push_back(new boost::thread(boost::bind(&ThreadPool::work, this)));
  }
}

ThreadPool::~ThreadPool()
{
  printf("Attempting to destroy the threadpool...\n");
  alive = false;
  for(unsigned int i=0; i<threads.size(); i++)
    schedule(PRIO_NORMAL, new NoWork());

  while(!threads.empty())
  {
    boost::thread* t = threads.front();
    threads.pop_front();
    t->join();
    delete t;
  }
  printf("Done destroying threadpool\n");
}

void ThreadPool::work()
{
  while(perform_one())
  {
  }

  printf("ThreadPool: Thread terminating...\n");
}

void ThreadPool::schedule(int priority, WorkInterface* wi)
{
  boost::unique_lock<boost::mutex> lock(mut);
  jobs[priority].push(wi);
  lock.unlock();
  jobcount.V();
}

void ThreadPool::schedule(int priority, boost::function<void ()> const& fn)
{
  schedule(priority, new BoostFunctionWork(fn));
}

void ThreadPool::cleanUp()
{
  boost::unique_lock<boost::mutex> lock(threadsMut);
  std::list<boost::thread*>::iterator cur = threads.begin();
  std::list<boost::thread*>::iterator end = threads.end();

  boost::thread::id nat; // represents not-a-thread
  printf("Cleanup started\n");
  
  while(cur!=end)
  {
    if((*cur)->get_id()==nat)
    {
      printf("Cleaning up finished thread\n");
      std::list<boost::thread*>::iterator tmp = cur;
      ++cur;
      delete *tmp;
      threads.erase(tmp);
    }
    else
    {
      ++cur;
    }
  }
  printf("Cleanup done\n");
}

bool ThreadPool::perform_one()
{
  jobcount.P();
  boost::unique_lock<boost::mutex> lock(mut);

  while(!jobs.empty() && jobs.begin()->second.empty())
    jobs.erase(jobs.begin());

  // At this point, either the jobs map is empty, or jobs.begin()->second contains tasks.
  
  if(!jobs.empty() && !jobs.begin()->second.empty())
  {
    int priority = jobs.begin()->first;
    WorkInterface* wi = jobs.begin()->second.front();
    jobs.begin()->second.pop();
    lock.unlock();

    bool result = wi->doWork();
    if(result)
    {
      schedule(priority, wi);
    }
    else
    {
      delete wi;
    }
  }
  else
  {
    printf("PANIC: JobQueue empty while it shouldn't be\n");
  }
  
  return alive;
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

bool QueueJumper::isDone()
{
  return !inQueue;
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

