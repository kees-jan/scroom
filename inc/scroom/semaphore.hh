/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

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
    boost::mutex::scoped_lock lock(mut);
    while(count==0)
    {
      cond.wait(lock);
    }
    count--;
  }

  inline void Semaphore::V()
  {
    boost::mutex::scoped_lock lock(mut);
    count++;
    cond.notify_one();
  }
}


#endif
