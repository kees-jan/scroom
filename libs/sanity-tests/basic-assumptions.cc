/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/weak_ptr.hpp>

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Basic_Assumptions)

BOOST_AUTO_TEST_CASE(weak_pointer_equality)
{
  {
    boost::weak_ptr<int> a;
    boost::weak_ptr<int> b;
    // Uninitialised weak pointers are equal
    BOOST_CHECK(!(a < b) && !(b < a));

    // Pointers to something that has been deleted are different from
    // uninitialized ones
    boost::shared_ptr<int> c = boost::shared_ptr<int>(new int());
    a                        = c;
    BOOST_CHECK((a < b) || (b < a));
    c.reset();
    BOOST_CHECK((a < b) || (b < a));

    // Pointers to something that has been deleted are still equal
    c = boost::shared_ptr<int>(new int());
    a = c;
    b = c;
    BOOST_CHECK(!(a < b) && !(b < a));
    c.reset();
    BOOST_CHECK(!(a < b) && !(b < a));
    BOOST_CHECK(!a.lock());
    BOOST_CHECK(!b.lock());
    BOOST_CHECK_EQUAL(a.lock(), b.lock());

    // Locking something that has been deleted equals an uninitialized pointer (?)
    boost::shared_ptr<int> d;
    BOOST_CHECK_EQUAL(a.lock(), d);
  }
}

BOOST_AUTO_TEST_SUITE_END()
