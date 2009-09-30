#include <stdio.h>

#include <boost/thread.hpp>

#include <scroom-semaphore.hh>

class ThreadPool
{
private:
  std::list<boost::thread*> threads;
  Scroom::Semaphore jobcount;

private:
  static void work();
  
public:
  ThreadPool();

  bool perform_one();
  
public:
  static ThreadPool& instance();
};



////////////////////////////////////////////////////////////////////////
/// ThreadPool
////////////////////////////////////////////////////////////////////////

static ThreadPool threadpool;

ThreadPool& ThreadPool::instance()
{
  return threadpool;
}

ThreadPool::ThreadPool()
{
  int count = boost::thread::hardware_concurrency();
  printf("Starting ThreadPool with %d threads\n", count);
  for(int i=0; i<count; i++)
  {
    threads.push_back(new boost::thread(work));
  }
}

void ThreadPool::work()
{
  while(instance().perform_one())
  {
  }

  printf("ThreadPool: Thread terminating...\n");
}

bool ThreadPool::perform_one()
{
  jobcount.P();
  
  return true;
}
