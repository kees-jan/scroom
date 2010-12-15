#include <scroom/threadpool.hh>

#include <stdio.h>

#include <threadpoolimpl.hh>

static ThreadPool& instance()
{
  static ThreadPool threadpool;

  return threadpool;
}

void schedule(WorkInterface* wi, int priority)
{
  instance().schedule(priority, wi);
}

void schedule_on_new_thread(WorkInterface* wi)
{
  instance().schedule_on_new_thread(wi);
}


