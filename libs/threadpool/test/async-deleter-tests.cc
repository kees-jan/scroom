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

#include <scroom/async-deleter.hh>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>

#include <scroom/semaphore.hh>

using namespace boost::posix_time;
using namespace Scroom;

const millisec short_timeout(250);
const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

class A
{
private:
  Semaphore* s;

public:
  A(Semaphore* s) : s(s) {}
  ~A() { s->V(); }
};

static void pass_and_destroy(Semaphore* toPass, boost::shared_ptr<A> toDestroy)
{
  toPass->P();
  toDestroy.reset();
}

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Async_Deleter_Tests)

BOOST_AUTO_TEST_CASE(deleter_deletes_asynchronously)
{
  Semaphore destroyed;
  boost::shared_ptr<A> a = boost::shared_ptr<A>(new A(&destroyed), AsyncDeleter<A>());
  BOOST_CHECK(!destroyed.P(short_timeout));

  Semaphore barrier;
  CpuBound()->schedule(boost::bind(pass_and_destroy, &barrier, a));
  BOOST_CHECK(!destroyed.P(short_timeout));
  a.reset();  
  BOOST_CHECK(!destroyed.P(short_timeout));
  barrier.V();
  BOOST_CHECK(destroyed.P(long_timeout));
}

BOOST_AUTO_TEST_SUITE_END()
