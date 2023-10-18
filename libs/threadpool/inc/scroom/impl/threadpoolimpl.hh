/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/threadpool.hh>

////////////////////////////////////////////////////////////////////////
// Code...

namespace Detail
{
  template <typename R, typename T>
  R threadPoolExecute(std::shared_ptr<T> fn)
  {
    return (*fn)();
  }
} // namespace Detail

template <typename T>
void ThreadPool::schedule(std::shared_ptr<T> fn, int priority, const ThreadPool::Queue::Ptr& queue)
{
  schedule(fn, priority, queue->getWeak());
}

template <typename T>
void ThreadPool::schedule(std::shared_ptr<T> fn, const ThreadPool::Queue::Ptr& queue)
{
  schedule(fn, defaultPriority, queue);
}

template <typename T>
void ThreadPool::schedule(std::shared_ptr<T> fn, int priority, ThreadPool::WeakQueue::Ptr queue)
{
  schedule([fn] { Detail::threadPoolExecute<void>(fn); }, priority, queue);
}

template <typename T>
void ThreadPool::schedule(std::shared_ptr<T> fn, ThreadPool::WeakQueue::Ptr queue)
{
  schedule(fn, defaultPriority, queue);
}

template <typename R>
boost::unique_future<R> ThreadPool::schedule(boost::function<R()> const& fn, int priority, const ThreadPool::Queue::Ptr& queue)
{
  return schedule(fn, priority, queue->getWeak());
}

template <typename R>
boost::unique_future<R> ThreadPool::schedule(boost::function<R()> const& fn, const ThreadPool::Queue::Ptr& queue)
{
  return schedule(fn, defaultPriority, queue);
}

template <typename R, typename T>
boost::unique_future<R> ThreadPool::schedule(std::shared_ptr<T> fn, int priority, const ThreadPool::Queue::Ptr& queue)
{
  return schedule<R, T>(fn, priority, queue->getWeak());
}

template <typename R, typename T>
boost::unique_future<R> ThreadPool::schedule(std::shared_ptr<T> fn, const ThreadPool::Queue::Ptr& queue)
{
  return schedule<R, T>(fn, defaultPriority, queue);
}

template <typename R>
boost::unique_future<R> ThreadPool::schedule(boost::function<R()> const& fn, int priority, ThreadPool::WeakQueue::Ptr queue)
{
  // Todo: If boost::function supported move semantics, we could do without
  // the shared pointer.

  // Todo: Without the static cast, Boost 1.53 packaged_task stores a
  // reference to fn, which is a temporary, and hence results in
  // undefined behaviour. Move semantics seem to work OK there...
  //
  // See https://svn.boost.org/trac/boost/ticket/8596
  std::shared_ptr<boost::packaged_task<R>> const t(new boost::packaged_task<R>(static_cast<boost::function<R()>>(fn)));
  boost::unique_future<R>                        f = t->get_future();
  schedule([t] { Detail::threadPoolExecute<void>(t); }, priority, queue);
  return f;
}

template <typename R>
boost::unique_future<R> ThreadPool::schedule(boost::function<R()> const& fn, ThreadPool::WeakQueue::Ptr queue)
{
  return schedule(fn, defaultPriority, queue);
}

template <typename R, typename T>
boost::unique_future<R> ThreadPool::schedule(std::shared_ptr<T> fn, int priority, ThreadPool::WeakQueue::Ptr queue)
{
  // Todo: If boost::function supported move semantics, we could do without
  // the shared pointer.
  const auto              t = std::make_shared<boost::packaged_task<R>>([fn] { return Detail::threadPoolExecute<R>(fn); });
  boost::unique_future<R> f = t->get_future();
  schedule([t] { Detail::threadPoolExecute<void>(t); }, priority, queue);
  return f;
}

template <typename R, typename T>
boost::unique_future<R> ThreadPool::schedule(std::shared_ptr<T> fn, ThreadPool::WeakQueue::Ptr queue)
{
  return schedule(fn, defaultPriority, queue);
}
