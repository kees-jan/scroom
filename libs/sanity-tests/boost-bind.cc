/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>


//////////////////////////////////////////////////////////////

class A
{
private:
  int& i;

public:
  typedef boost::shared_ptr<A> Ptr;
  
  A(int& i)
    :i(i)
  {}
  
  void set(int v)
  {
    i=v;
  }

  int get()
  {
    return i;
  }
  
  static Ptr create(int& i)
  {
    return Ptr(new A(i));
  }
};


//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(boost_bind_Tests)

BOOST_AUTO_TEST_CASE(keeps_object_alive_while_setting)
{
  int value=25;
  const int expected = 1;
  A::Ptr a = A::create(value);
  boost::function<void ()> f = boost::bind(&A::set, a, expected);

  a.reset();
  f();
  BOOST_CHECK_EQUAL(expected, value);
}

BOOST_AUTO_TEST_CASE(keeps_object_alive_while_getting)
{
  int value=25;
  const int expected = value;
  A::Ptr a = A::create(value);
  boost::function<int ()> f = boost::bind(&A::get, a);

  a.reset();
  BOOST_CHECK_EQUAL(expected, f());
}


BOOST_AUTO_TEST_SUITE_END()
