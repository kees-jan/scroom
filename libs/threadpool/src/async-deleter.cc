/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/async-deleter.hh>
#include <scroom/threadpool.hh>

namespace Scroom
{
  namespace Detail
  {
    namespace ThreadPool
    {
      ::ThreadPool::Ptr getDeleter()
      {
        static ::ThreadPool::Ptr deleter = ::ThreadPool::Ptr(new ::ThreadPool(1, true));
        return deleter;
      }
    } // namespace ThreadPool
  }   // namespace Detail
} // namespace Scroom
