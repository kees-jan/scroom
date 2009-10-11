#include <threadpool.hh>

#include <stdio.h>

#include <boost/thread.hpp>
#include <queue>
#include <map>

#include <scroom-semaphore.hh>

#include <workinterface.hh>

class NoWork : public WorkInterface
{
public:
  virtual bool doWork();
};

class CleanUp : public WorkInterface
{
public:
  virtual bool doWork();
};

class ThreadPool
{
private:
  std::list<boost::thread*> threads;
  Scroom::Semaphore jobcount;
  std::map<int, std::queue<WorkInterface*> > jobs;
  boost::mutex mut;
  boost::mutex threadsMut;
  bool alive;

private:
  static void work();
  ThreadPool();
  ~ThreadPool();
  bool perform_one();
  
public:
  void schedule(int priority, WorkInterface* wi);
  void schedule_on_new_thread(WorkInterface* wi);

  void cleanUp();
    
public:
  static ThreadPool& instance();
};

class BoostWrapper
{
private:
  WorkInterface* wi;
public:
  BoostWrapper(WorkInterface* wi);
  void operator()();
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

void ThreadPool::schedule(int priority, WorkInterface* wi)
{
  boost::unique_lock<boost::mutex> lock(mut);
  jobs[priority].push(wi);
  lock.unlock();
  jobcount.V();
}

void ThreadPool::schedule_on_new_thread(WorkInterface* wi)
{
  boost::unique_lock<boost::mutex> lock(threadsMut);
  threads.push_back(new boost::thread(BoostWrapper(wi)));
}

void ThreadPool::cleanUp()
{
  boost::unique_lock<boost::mutex> lock(threadsMut);
  std::list<boost::thread*>::iterator cur = threads.begin();
  std::list<boost::thread*>::iterator end = threads.end();

  boost::thread::id nat; // represents not-a-thread

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
/// NoWork
////////////////////////////////////////////////////////////////////////

bool NoWork::doWork()
{
  printf("Executing dummy job...\n");
  return false;
}

////////////////////////////////////////////////////////////////////////
/// CleanUp
////////////////////////////////////////////////////////////////////////

bool CleanUp::doWork()
{
  ThreadPool::instance().cleanUp();
  return false;
}

////////////////////////////////////////////////////////////////////////
/// BoostWrapper
////////////////////////////////////////////////////////////////////////

BoostWrapper::BoostWrapper(WorkInterface* wi)
  : wi(wi)
{
}

void BoostWrapper::operator()()
{
  while(wi->doWork())
  {
    // repeat
  }
  delete wi;
  schedule(new CleanUp(), PRIO_HIGHEST);
}

////////////////////////////////////////////////////////////////////////
/// C-style functions
////////////////////////////////////////////////////////////////////////

void schedule(WorkInterface* wi, int priority)
{
  ThreadPool::instance().schedule(priority, wi);
}

void schedule_on_new_thread(WorkInterface* wi)
{
  ThreadPool::instance().schedule_on_new_thread(wi);
}
