/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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

    template<typename duration_type>
    bool P(duration_type const& rel_time);

    bool try_P();
  };

  inline Semaphore::Semaphore(unsigned int count_)
    :count(count_)
  {
  }

  inline void Semaphore::P()
  {
    boost::mutex::scoped_lock lock(mut);
    while(count==0)
    {
      cond.wait(lock);
    }
    count--;
  }

  inline bool Semaphore::try_P()
  {
    boost::mutex::scoped_lock lock(mut);
    if(count>0)
    {
      count--;
      return true;
    }

    return false;
  }

  template<typename duration_type>
  inline bool Semaphore::P(duration_type const& rel_time)
  {
    boost::posix_time::ptime timeout = boost::posix_time::second_clock::universal_time() + rel_time;

    boost::mutex::scoped_lock lock(mut);
    while(count==0)
    {
      if(!cond.timed_wait(lock, timeout))
        return false;
    }
    count--;
    return true;
  }

  inline void Semaphore::V()
  {
    boost::mutex::scoped_lock lock(mut);
    count++;
    cond.notify_one();
  }
}

