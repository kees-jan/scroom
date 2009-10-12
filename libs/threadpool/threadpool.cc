#include <threadpool.hh>

#include <stdio.h>

#include <boost/thread.hpp>
#include <queue>
#include <map>

#include <workinterface.hh>

class NoWork : public WorkInterface
{
public:
  virtual bool doWork();
};

class GcCleanUp : public WorkInterface
{
public:
  virtual bool doWork();
};

class QjCleanUp : public WorkInterface
{
private:
  QueueJumper* qj;

public:
  QjCleanUp(QueueJumper* qj);
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
/// NoWork
////////////////////////////////////////////////////////////////////////

bool NoWork::doWork()
{
  printf("Executing dummy job...\n");
  return false;
}

////////////////////////////////////////////////////////////////////////
/// GcCleanUp
////////////////////////////////////////////////////////////////////////

bool GcCleanUp::doWork()
{
  ThreadPool::instance().cleanUp();
  return false;
}

////////////////////////////////////////////////////////////////////////
/// QjCleanUp
////////////////////////////////////////////////////////////////////////

QjCleanUp::QjCleanUp(QueueJumper* qj)
  :qj(qj)
{
}

bool QjCleanUp::doWork()
{
  qj->waitForDone();
  delete qj;
  qj=NULL;
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
  schedule(new GcCleanUp(), PRIO_HIGHEST);
  printf("Thread terminating. Cleanup scheduled...\n");
}

////////////////////////////////////////////////////////////////////////
/// QueueJumper
////////////////////////////////////////////////////////////////////////

QueueJumper::QueueJumper()
  : wi(NULL), mut(), inQueue(true), priority(PRIO_NORMAL)
{}

bool QueueJumper::setWork(WorkInterface* wi)
{
  boost::unique_lock<boost::mutex> lock(mut);
  if(inQueue)
  {
    // Our turn hasn't passed yet. Accept work.
    this->wi = wi;
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

void QueueJumper::waitForDone()
{
  done.P();
}

void QueueJumper::asynchronousCleanup()
{
  schedule_on_new_thread(new QjCleanUp(this));
}

void QueueJumper::schedule(int priority)
{
  this->priority = priority;
  ::schedule(new Wrapper(this), priority);
}

bool QueueJumper::doWork()
{
  boost::unique_lock<boost::mutex> lock(mut);
  if(wi)
  {
    if(wi->doWork())
      ::schedule(wi, priority);
    else
      delete wi;
  }
  inQueue=false;
  lock.unlock();
  done.V();
  return false;
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
