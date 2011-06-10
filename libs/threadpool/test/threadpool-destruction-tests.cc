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

#include <scroom/threadpool.hh>

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

BOOST_AUTO_TEST_SUITE(ThreadPool_destruction_Tests)

BOOST_AUTO_TEST_CASE(threads_terminate_on_destruction)
{
  ThreadPool* pool = new ThreadPool(0);
  ThreadPool::ThreadPtr t = pool->add();
  BOOST_CHECK(!t->timed_join(short_timeout));
  delete pool;

  bool success = boost::thread::id() == t->get_id();
  BOOST_CHECK(success);
  if(!success)
  {
    t->interrupt();
    t->timed_join(long_timeout);
    BOOST_REQUIRE(boost::thread::id() == t->get_id());
  }
}

BOOST_AUTO_TEST_CASE(threads_can_be_interrupted)
{
  ThreadPool pool(0);
  ThreadPool::ThreadPtr t = pool.add();
  boost::this_thread::sleep(millisec(50));
  t->interrupt();
  t->timed_join(short_timeout);
  BOOST_CHECK_EQUAL(boost::thread::id(), t->get_id());
}

BOOST_AUTO_TEST_CASE(destroy_threadpool_with_nonempty_queue)
{
  ThreadPool* pool = new ThreadPool(0);
  Semaphore dummy(0);

  pool->schedule(boost::bind(clear_sem, &dummy));

  boost::thread t(boost::bind(destroy, pool));
  t.timed_join(long_timeout);
  BOOST_REQUIRE(boost::thread::id() == t.get_id());
}

BOOST_AUTO_TEST_CASE(cant_destroy_threadpool_with_running_job)
{
  ThreadPool* pool = new ThreadPool(1);
  Semaphore a(0);
  Semaphore b(0);

  pool->schedule(boost::bind(pass_and_clear, &a, &b));

  // Give the thread some time to start the job
  boost::this_thread::sleep(millisec(50));
  
  boost::thread t(boost::bind(destroy, pool));
  BOOST_CHECK(!t.timed_join(short_timeout));
  a.V();
  t.timed_join(long_timeout);
  BOOST_REQUIRE(boost::thread::id() == t.get_id());
  BOOST_CHECK(b.P(short_timeout));
}



BOOST_AUTO_TEST_SUITE_END()
