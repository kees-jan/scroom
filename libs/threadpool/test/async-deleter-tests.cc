/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
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

#include "helpers.hh"
#include "scroom/function-additor.hh"

using namespace boost::posix_time;
using namespace Scroom;

static const millisec short_timeout(250);
static const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

class A
{
private:
  Semaphore* s1;
  Semaphore* s2;

public:
  A(Semaphore* s1_, Semaphore* s2_) : s1(s1_), s2(s2_) {}
  ~A() { s1->P(); s2->V(); }
};

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Async_Deleter_Tests)

BOOST_AUTO_TEST_CASE(deleter_deletes_asynchronously)
{
  Semaphore barrier1;
  Semaphore destroyed;
  boost::shared_ptr<A> a = boost::shared_ptr<A>(new A(&barrier1, &destroyed), AsyncDeleter<A>());
  BOOST_CHECK(!destroyed.P(short_timeout));

  Semaphore barrier2;
  Semaphore signal;
  CpuBound()->schedule(pass(&barrier2)+destroy(a)+clear(&signal));
  a.reset();
  barrier2.V();
  BOOST_CHECK(signal.P(long_timeout));
  BOOST_CHECK(!destroyed.P(short_timeout));
  barrier1.V();
  BOOST_CHECK(destroyed.P(long_timeout));
}

BOOST_AUTO_TEST_SUITE_END()
