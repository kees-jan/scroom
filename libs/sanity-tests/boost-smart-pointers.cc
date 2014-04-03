/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

#include <boost/test/unit_test.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

//////////////////////////////////////////////////////////////

namespace
{
  class A
  {
  public:
    typedef boost::shared_ptr<A> Ptr;
    typedef boost::weak_ptr<A> WeakPtr;

    void whatever() {}

    static Ptr create() { return Ptr(new A()); }
  };
}

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Boost_Smart_Pointer_Tests)

BOOST_AUTO_TEST_CASE(bind_copies_smart_pointer)
{
  A::Ptr a = A::create();
  BOOST_CHECK(a);
  A::WeakPtr a_weak = a;
  BOOST_CHECK(a_weak.lock());

  boost::function<void ()> fn = boost::bind(&A::whatever, a);
  a.reset();
  BOOST_CHECK(!a);
  BOOST_CHECK(a_weak.lock());
}

BOOST_AUTO_TEST_SUITE_END()
