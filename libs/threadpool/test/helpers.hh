#ifndef HELPERS_HH
#define HELPERS_HH

#include <scroom/semaphore.hh>
#include <scroom/threadpool.hh>

using namespace Scroom;

void pass_destroy_and_clear(Semaphore* s1, ThreadPool::Queue::WeakPtr q, Semaphore* s2);
void clear_sem(Semaphore* s);
void clear_and_pass(Semaphore* toClear, Semaphore* toPass);


#endif
