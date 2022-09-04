/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/threadpool.hh>

namespace
{
  template <typename T>
  void call_delete(T* p)
  {
    delete p;
  }
} // namespace

namespace Scroom::Detail::ThreadPool
{
  ::ThreadPool::Ptr getDeleter();
} // namespace Scroom::Detail::ThreadPool

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
 */
template <typename T>
class AsyncDeleter
{
private:
  ThreadPool::Ptr deleter;

public:
  AsyncDeleter()
    : deleter(Scroom::Detail::ThreadPool::getDeleter())
  {
  }

  void operator()(T* p) { deleter->schedule(boost::bind(&call_delete<T>, p)); }
};
