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

#include <threadpool.hh>

#include <stdio.h>

#include <boost/thread.hpp>
#include <queue>

#include <scroom-semaphore.hh>

#include <workinterface.hh>

struct Job
{
public:
  int priority;
  WorkInterface* wi;

public:
  Job(int priority, WorkInterface* wi)
    : priority(priority), wi(wi)
  {
  }

  bool operator< (const Job& other) const
  {
    return priority < other.priority;
  }
};

class NoWork : public WorkInterface
{
public:
  virtual bool doWork();
};

class ThreadPool
{
private:
  std::list<boost::thread*> threads;
  Scroom::Semaphore jobcount;
  std::priority_queue<Job> jobs;
  boost::mutex mut;
  bool alive;

private:
  static void work();
  ThreadPool();
  ~ThreadPool();
  bool perform_one();
  
public:
  void schedule(Job j);
  void schedule(int priority, WorkInterface* wi);
  
public:
  static ThreadPool& instance();
};


////////////////////////////////////////////////////////////////////////
/// ThreadPool
////////////////////////////////////////////////////////////////////////

ThreadPool& ThreadPool::instance()
{
  static ThreadPool threadpool;

  return threadpool;
}

ThreadPool::ThreadPool()
{
  int count = std::max(boost::thread::hardware_concurrency(),(unsigned int)2);
  alive=true;
  printf("Starting ThreadPool with %d threads\n", count);
  for(int i=0; i<count; i++)
  {
    threads.push_back(new boost::thread(work));
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
  while(instance().perform_one())
  {
  }

  printf("ThreadPool: Thread terminating...\n");
}

void ThreadPool::schedule(Job j)
{
  boost::unique_lock<boost::mutex> lock(mut);
  jobs.push(j);
  lock.unlock();
  jobcount.V();
}

void ThreadPool::schedule(int priority, WorkInterface* wi)
{
  schedule(Job(priority, wi));
}

bool ThreadPool::perform_one()
{
  jobcount.P();
  boost::unique_lock<boost::mutex> lock(mut);
  if(!jobs.empty())
  {
    Job j = jobs.top();
    jobs.pop();
    lock.unlock();

    bool result = j.wi->doWork();
    if(result)
    {
      schedule(j);
    }
    else
    {
      delete j.wi;
    }
  }
  else
  {
    printf("PANIC: JobQueue empty while it shouldn't be\n");
  }
  
  return alive;
}

////////////////////////////////////////////////////////////////////////
/// NoWork
////////////////////////////////////////////////////////////////////////

bool NoWork::doWork()
{
  return false;
}

////////////////////////////////////////////////////////////////////////
/// C-style functions
////////////////////////////////////////////////////////////////////////

void schedule(WorkInterface* wi, int priority)
{
  ThreadPool::instance().schedule(Job(priority, wi));
}
