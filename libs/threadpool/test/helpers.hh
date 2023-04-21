/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>

#include <boost/function.hpp>

#include <scroom/semaphore.hh>
#include <scroom/threadpool.hh>

using namespace Scroom;

boost::function<void()> pass(Semaphore* s);
boost::function<void()> clear(Semaphore* s);
boost::function<void()> destroy(std::shared_ptr<void>& p);

template <typename T>
boost::function<void()> destroy(std::shared_ptr<T>& p)
{
  std::shared_ptr<void> pv = std::move(p);
  return destroy(pv);
}