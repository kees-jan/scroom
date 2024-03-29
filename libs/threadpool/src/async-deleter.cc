/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <memory>

#include <scroom/async-deleter.hh>
#include <scroom/threadpool.hh>

namespace Scroom::Detail::ThreadPool
{
  ::ThreadPool::Ptr getDeleter()
  {
    static ::ThreadPool::Ptr const deleter = std::make_shared<::ThreadPool>(1, true);
    return deleter;
  }
} // namespace Scroom::Detail::ThreadPool
