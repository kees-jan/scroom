/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "helpers.hh"

#include <utility>

#include <scroom/semaphore.hh>

//////////////////////////////////////////////////////////////

namespace
{

} // namespace

//////////////////////////////////////////////////////////////

boost::function<void()> pass(Semaphore* s)
{
  return [s] { s->P(); };
}

boost::function<void()> clear(Semaphore* s)
{
  return [s] { s->V(); };
}

boost::function<void()> destroy(std::shared_ptr<void>& p)
{
  return [p = std::move(p)]() mutable { p.reset(); };
}
