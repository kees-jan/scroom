#ifndef THREADPOOLIMPL_HH
#define THREADPOOLIMPL_HH

#include <queue>

#include <boost/thread.hpp>

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

class ThreadPool
{
private:
  std::list<boost::thread*> threads;
  Scroom::Semaphore jobcount;
  std::priority_queue<Job> jobs;
  boost::mutex mut;
  bool alive;

private:
  void work();
  bool perform_one();
  
public:
  ThreadPool();
  ~ThreadPool();
  void schedule(Job j);
  void schedule(int priority, WorkInterface* wi);
};

#endif
