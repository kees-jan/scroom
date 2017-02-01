/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <type_traits>

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/parameterized_test.hpp>


#include <scroom/colormappable.hh>
#include <scroom/unused.hh>

////////////////////////////////////////////////////////////////////////
// Begin Yuk
// See http://stackoverflow.com/a/38642890

#if BOOST_VERSION > 105800
#define MY_BOOST_TEST_ADD_ARGS __FILE__, __LINE__,
#define MY_BOOST_TEST_DEFAULT_DEC_COLLECTOR ,boost::unit_test::decorator::collector::instance()
#else
#define MY_BOOST_TEST_ADD_ARGS
#define MY_BOOST_TEST_DEFAULT_DEC_COLLECTOR
#endif

#define BOOST_FIXTURE_PARAM_TEST_CASE( test_name, F, mbegin, mend )     \
struct test_name : public F {                                           \
   typedef ::std::remove_const< ::std::remove_reference< decltype(*(mbegin)) >::type>::type param_t; \
   void test_method(const param_t &);                                   \
};                                                                      \
                                                                        \
void BOOST_AUTO_TC_INVOKER( test_name )(const test_name::param_t &param) \
{                                                                       \
    test_name t;                                                        \
    t.test_method(param);                                               \
}                                                                       \
                                                                        \
BOOST_AUTO_TU_REGISTRAR( test_name )(                                   \
    boost::unit_test::make_test_case(                                   \
       &BOOST_AUTO_TC_INVOKER( test_name ), #test_name,                 \
       MY_BOOST_TEST_ADD_ARGS                                           \
       (mbegin), (mend))                                                \
       MY_BOOST_TEST_DEFAULT_DEC_COLLECTOR);                            \
                                                                        \
void test_name::test_method(const param_t &param)                       \

#define BOOST_AUTO_PARAM_TEST_CASE( test_name, mbegin, mend )           \
   BOOST_FIXTURE_PARAM_TEST_CASE( test_name,                            \
                                  BOOST_AUTO_TEST_CASE_FIXTURE,         \
                                  mbegin, mend)

// End Yuk
////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(ColormapHelper_Tests)

// After creation, originalColormap and colormap are both set and equal

BOOST_AUTO_TEST_CASE(create_helper)
{
  ColormapHelper::Ptr helper = ColormapHelper::create(4);
  Colormap::Ptr defaultColormap = helper->getColormap();
  BOOST_CHECK_EQUAL(4, defaultColormap->colors.size());
  BOOST_CHECK_EQUAL(4, helper->getNumberOfColors());
}

BOOST_AUTO_TEST_CASE(create_monochrome_helper)
{
  MonochromeColormapHelper::Ptr helper = MonochromeColormapHelper::create(256);
  Colormap::Ptr defaultColormap = helper->getColormap();
  BOOST_CHECK_EQUAL(256, defaultColormap->colors.size());
  BOOST_CHECK_EQUAL(256, helper->getNumberOfColors());
}

struct Data
{
  ColormapHelper::Ptr helper;
  int expectedColors;

  Data(int expectedColors, ColormapHelper::Ptr const& helper)
    : helper(helper), expectedColors(expectedColors)
  {}
};

static std::list<Data> helpers = boost::assign::list_of
  (Data(4, ColormapHelper::create(4)))
  (Data(2, ColormapHelper::create(Colormap::createDefault(2))))
  (Data(4, ColormapHelper::createInverted(4)))
  ;

BOOST_AUTO_PARAM_TEST_CASE(colormaps_equal_and_correct_count, helpers.begin(), helpers.end())
{
  Colormap::Ptr originalColormap = param.helper->getOriginalColormap();
  BOOST_REQUIRE(originalColormap);
  BOOST_CHECK_EQUAL(param.expectedColors, originalColormap->colors.size());
  
  Colormap::Ptr colormap = param.helper->getColormap();
  BOOST_REQUIRE(colormap);
  BOOST_CHECK_EQUAL(param.expectedColors, colormap->colors.size());
  BOOST_CHECK_EQUAL(originalColormap, colormap);
}

BOOST_AUTO_TEST_SUITE_END()
