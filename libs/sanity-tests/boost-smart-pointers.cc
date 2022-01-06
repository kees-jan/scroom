/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/weak_ptr.hpp>

//////////////////////////////////////////////////////////////

namespace
{
  class A
  {
  public:
    using Ptr     = boost::shared_ptr<A>;
    using WeakPtr = boost::weak_ptr<A>;

    void whatever() {}

    static Ptr create() { return Ptr(new A()); }
  };
} // namespace

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Boost_Smart_Pointer_Tests)

BOOST_AUTO_TEST_CASE(bind_copies_smart_pointer)
{
  A::Ptr a = A::create();
  BOOST_CHECK(a);
  A::WeakPtr a_weak = a;
  BOOST_CHECK(a_weak.lock());

  boost::function<void()> fn = boost::bind(&A::whatever, a);
  a.reset();
  BOOST_CHECK(!a);
  BOOST_CHECK(a_weak.lock());
}

BOOST_AUTO_TEST_SUITE_END()
