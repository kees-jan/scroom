/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "helpers.hh"

#include <scroom/semaphore.hh>

//////////////////////////////////////////////////////////////

namespace
{
  void passImpl(Semaphore* s) { s->P(); }

  void clearImpl(Semaphore* s) { s->V(); }

  void destroyImpl(boost::shared_ptr<void>& p) { p.reset(); }
} // namespace

//////////////////////////////////////////////////////////////

boost::function<void()> pass(Semaphore* s) { return boost::bind(passImpl, s); }

boost::function<void()> clear(Semaphore* s) { return boost::bind(clearImpl, s); }

boost::function<void()> destroy(boost::shared_ptr<void> p) { return boost::bind(destroyImpl, p); }
