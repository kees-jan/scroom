#ifndef _SCROOM_SEMAPHORE_HH
#define _SCROOM_SEMAPHORE_HH

#include <boost/thread.hpp>

namespace Scroom
{
  class Semaphore
  {
  private:
    unsigned int count;
    boost::condition_variable cond;
    boost::mutex mut;

  public:
    Semaphore(unsigned int count=0);
    void P();
    void V();
  };

  inline Semaphore::Semaphore(unsigned int count)
    :count(count)
  {
  }

  inline void Semaphore::P()
  {
    boost::unique_lock<boost::mutex> lock(mut);
    while(count==0)
    {
      cond.wait(lock);
    }
    count--;
  }

  inline void Semaphore::V()
  {
    boost::unique_lock<boost::mutex> lock(mut);
    count++;
    cond.notify_one();
  }
}


#endif
