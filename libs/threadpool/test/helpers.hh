/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <scroom/semaphore.hh>
#include <scroom/threadpool.hh>

using namespace Scroom;

boost::function<void ()> pass(Semaphore* s);
boost::function<void ()> clear(Semaphore* s);
boost::function<void ()> destroy(boost::shared_ptr<void> p);




