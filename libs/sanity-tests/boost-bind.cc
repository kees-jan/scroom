/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
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

  A(int& i_)
    :i(i_)
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
