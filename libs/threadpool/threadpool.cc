#include <threadpool.hh>

#include <threadpoolimpl.hh>

static ThreadPool& instance()
{
  static ThreadPool threadpool;

  return threadpool;
}

void schedule(WorkInterface* wi, int priority)
{
  instance().schedule(Job(priority, wi));
}
