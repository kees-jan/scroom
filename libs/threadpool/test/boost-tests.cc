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

#include "helpers.hh"
#include "function-additor.hh"

using namespace boost::posix_time;
using namespace Scroom;

const millisec short_timeout(250);
const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

void test_interrupts1(Semaphore* s1, Semaphore* s2, Semaphore* s3, Semaphore* s4)
{
  try
  {
    s1->V();
    s2->P();
    s3->V();
  }
  catch(boost::thread_interrupted& ex)
  {
    s4->V();
  }
  
}

void test_interrupts2(Semaphore* s1, Semaphore* s2, bool interruption_point,
                      Semaphore* s3, Semaphore* s4)
{
  try
  {
    {
      boost::this_thread::disable_interruption for_now;
      s1->V();
      s2->P();
    }

    if(interruption_point)
      boost::this_thread::interruption_point();
    
    s3->V();
  }
  catch(boost::thread_interrupted& ex)
  {
    s4->V();
  }
}

//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Boost_Tests)

BOOST_AUTO_TEST_SUITE(Interruption_Tests)

BOOST_AUTO_TEST_CASE(interrupts_interrupt1)
{
  Semaphore s1(0);
  Semaphore s2(0);
  Semaphore s3(0);
  Semaphore s4(0);

  boost::thread t(boost::bind(test_interrupts1, &s1, &s2, &s3, &s4));
  BOOST_CHECK(s1.P(long_timeout));
  boost::this_thread::sleep(short_timeout);
  t.interrupt();
  BOOST_CHECK(s4.P(long_timeout));
  BOOST_CHECK(!s3.try_P());

  t.timed_join(long_timeout);
  BOOST_CHECK_EQUAL(boost::thread::id(), t.get_id());
}

BOOST_AUTO_TEST_CASE(interrupts_interrupt2a)
{
  Semaphore s1(0);
  Semaphore s2(0);
  Semaphore s3(0);
  Semaphore s4(0);

  boost::thread t(boost::bind(test_interrupts2, &s1, &s2, false, &s3, &s4));
  BOOST_CHECK(s1.P(long_timeout));
  boost::this_thread::sleep(short_timeout);
  t.interrupt();
  BOOST_CHECK(!t.timed_join(short_timeout));
  s2.V();
  BOOST_CHECK(s3.P(long_timeout));
  BOOST_CHECK(!s4.try_P());

  t.timed_join(long_timeout);
  BOOST_CHECK_EQUAL(boost::thread::id(), t.get_id());
}

BOOST_AUTO_TEST_CASE(interrupts_interrupt2)
{
  Semaphore s1(0);
  Semaphore s2(0);
  Semaphore s3(0);
  Semaphore s4(0);

  boost::thread t(boost::bind(test_interrupts2, &s1, &s2, true, &s3, &s4));
  BOOST_CHECK(s1.P(long_timeout));
  boost::this_thread::sleep(short_timeout);
  t.interrupt();
  BOOST_CHECK(!t.timed_join(short_timeout));
  s2.V();
  BOOST_CHECK(s4.P(long_timeout));
  BOOST_CHECK(!s3.try_P());

  t.timed_join(long_timeout);
  BOOST_CHECK_EQUAL(boost::thread::id(), t.get_id());
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
