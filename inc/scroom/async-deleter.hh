/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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

#ifndef ASYNCDELETER_HH_
#define ASYNCDELETER_HH_

#include <scroom/threadpool.hh>

namespace
{
  template<typename T>
  void call_delete(T* p)
  {
    delete p;
  }
}

namespace Scroom
{
  namespace Detail
  {
    namespace ThreadPool
    {
      ::ThreadPool::Ptr getDeleter();
    }
  }
}


/**
 * Allow boost shared pointers to asynchronously delete their targets.
 *
 * When some objects last shared pointer goes out of scope, the object
 * is deleted. Usually, this is done on the thread that destroys the
 * last pointer. This class allows destruction to be done on a
 * separate thread.
 *
 * This is especially advantageous if deleting the object takes a long
 * time and there is no immediate hurry. Deleting ThreadPool::Queue
 * objects, for example, blocks as long as a thread is currently
 * executing a job on the queue. This might take some time you do not
 * wish to wait.
 *
 * @warning During program termination, the last AsyncDeleter object
 * has the responsibility of destroying the thread that is used for
 * deleting. As a result, the current thread may block until the
 * deleter thread finishes deleting the object. So you could say that
 * the last object is deleted synchronously. Hence it is not a good
 * idea to us AsyncDeleter to avoid deadlocks, unless you cn be sure
 * that all objects will be deleted before the program terminates.
 */
template<typename T>
class AsyncDeleter
{
private:
  ThreadPool::Ptr deleter;

public:
  AsyncDeleter()
    : deleter(Scroom::Detail::ThreadPool::getDeleter())
  {}

  ~AsyncDeleter() {}

  void operator()(T* p)
  {
    deleter->schedule(boost::bind(&call_delete<T>, p));
  }
};

#endif /* ASYNCDELETER_HH_ */
