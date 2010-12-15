#ifndef THREADPOOLIMPL_HH
#define THREADPOOLIMPL_HH

#include <queue>

#include <boost/thread.hpp>
#include <boost/function.hpp>

#include <scroom/semaphore.hh>

#include <scroom/workinterface.hh>

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
  void work();
  bool perform_one();
  
public:
  ThreadPool();
  ThreadPool(int count);
  ~ThreadPool();
  void schedule(int priority, WorkInterface* wi);
  void schedule(int priority, boost::function<void ()> const& fn);
  void schedule_on_new_thread(WorkInterface* wi);
  void cleanUp();
};

#endif
