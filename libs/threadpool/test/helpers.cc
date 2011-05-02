#include "helpers.hh"

//////////////////////////////////////////////////////////////

void pass_destroy_and_clear(Semaphore* s1, ThreadPool::Queue::WeakPtr q, Semaphore* s2)
{
  ThreadPool::Queue::Ptr queue(q);
  s1->P();
  queue.reset();
  s2->V();
}

void clear_sem(Semaphore* s)
{
  s->V();
}

void clear_and_pass(Semaphore* toClear, Semaphore* toPass)
{
  toClear->V();
  toPass->P();
}

