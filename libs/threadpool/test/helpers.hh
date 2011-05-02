#ifndef HELPERS_HH
#define HELPERS_HH

#include <scroom/semaphore.hh>
#include <scroom/threadpool.hh>

using namespace Scroom;

void pass_destroy_and_clear(Semaphore* s0, Semaphore* s1, Semaphore* s2, ThreadPool::Queue::WeakPtr q);
void clear_sem(Semaphore* s);
void clear_and_pass(Semaphore* toClear, Semaphore* toPass);


#endif
