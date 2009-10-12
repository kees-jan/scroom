#ifndef _THREADPOOL_HH
#define _THREADPOOL_HH

#include <boost/thread.hpp>

#include <scroom-semaphore.hh>
#include <workinterface.hh>

enum
  {
    PRIO_HIGHEST = 100,
    PRIO_HIGHER,
    PRIO_HIGH,
    PRIO_NORMAL,
    PRIO_LOW,
    PRIO_LOWER,
    PRIO_LOWEST,
  };

class Wrapper : public WorkInterface
{
private:
  WorkInterface* wi;

public:
  Wrapper(WorkInterface* wi)
    : wi(wi)
  {}

  virtual bool doWork()
  {
    return wi->doWork();
  }
};

class QueueJumper : private WorkInterface
{
private:
  WorkInterface* wi;
  boost::mutex mut;
  bool inQueue;
  int priority;
  Scroom::Semaphore done;

public:
  QueueJumper();

  bool setWork(WorkInterface* wi);
  bool isDone();
  void waitForDone();
  void asynchronousCleanup();

  void schedule(int priority=PRIO_NORMAL);
  
private:
  virtual bool doWork();
};


void schedule(WorkInterface* wi, int priority=PRIO_NORMAL);

void schedule_on_new_thread(WorkInterface* wi);

#endif
