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

#ifndef _IMPL_THREADPOOL_HH
#define _IMPL_THREADPOOL_HH

////////////////////////////////////////////////////////////////////////
// Code...

namespace
{
  template<typename R, typename T>
  R threadPoolExecute(boost::shared_ptr<T> fn)
  {
    return (*fn)();
  }
}

template<typename T>
void ThreadPool::schedule(boost::shared_ptr<T> fn, int priority)
{
  schedule(boost::bind(threadPoolExecute<void, T>, fn), priority);
}

#ifdef NEW_BOOST_FUTURES
#ifdef HAVE_STDCXX_0X
template<typename R>
boost::unique_future<R>  ThreadPool::schedule(boost::function<R ()> const& fn, int priority)
{
  // Todo: If boost::function supported move semantics, we could do without
  // the shared pointer.
  boost::shared_ptr<boost::packaged_task<R>> t(new boost::packaged_task<R>(fn));
  boost::unique_future<R> f = t->get_future();
  schedule(boost::bind(threadPoolExecute<void, boost::packaged_task<R>>, t), priority);
  return f;
}

template<typename R, typename T>
boost::unique_future<R> ThreadPool::schedule(boost::shared_ptr<T> fn, int priority)
{
  // Todo: If boost::function supported move semantics, we could do without
  // the shared pointer.
  boost::shared_ptr<boost::packaged_task<R>> t(new boost::packaged_task<R>(boost::bind(threadPoolExecute<R,T>, fn)));
  boost::unique_future<R> f = t->get_future();
  schedule(boost::bind(threadPoolExecute<void, boost::packaged_task<R>>, t), priority);
  return f;
}
#endif /* HAVE_STDCXX_0X */
#else /* NEW_BOOST_FUTURES */

template<typename R>
boost::future<R> ThreadPool::schedule(boost::function<R ()> const& fn, int priority)
{
  boost::promise<R> prom; // create promise
  schedule(boost::future_wrapper<R>(fn, prom), priority); //queue the job
  return boost::future<R>(prom); // return a future created from the promise
}

template<typename R, typename T>
boost::future<R> ThreadPool::schedule(boost::shared_ptr<T> fn, int priority)
{
  boost::promise<R> prom; // create promise
  schedule(boost::future_wrapper<R>(boost::bind(threadPoolExecute<R,T>, fn), prom), priority); //queue the job
  return boost::future<R>(prom); // return a future created from the promise
}
#endif /* NEW_BOOST_FUTURES */

namespace CpuBound
{
  template<typename T>
  void schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL)
  {
    schedule(boost::bind(threadPoolExecute<void, T>, fn), priority);
  }
}

namespace Sequentially
{
  template<typename T>
  void schedule(boost::shared_ptr<T> fn, int priority=PRIO_NORMAL)
  {
    schedule(boost::bind(threadPoolExecute<void, T>, fn), priority);
  }
}

#endif
