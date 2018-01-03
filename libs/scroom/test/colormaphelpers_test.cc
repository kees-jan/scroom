/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
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

namespace
{
  const Color Blue(0,0,1);
  const double accuracy = 1e-4;
}
  
BOOST_AUTO_TEST_SUITE(ColormapHelper_Tests)

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
  (Data(256, MonochromeColormapHelper::create(256)))
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

BOOST_AUTO_TEST_CASE(regular_colormaps_cant_have_their_colors_set)
{
  ColormapHelper::Ptr helper = ColormapHelper::create(256);
  BOOST_CHECK_THROW(helper->setMonochromeColor(Color(0,0,1)), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(monochrome_colormap_can_have_its_color_set)
{
  ColormapHelper::Ptr helper = MonochromeColormapHelper::create(256);
  Colormap::Ptr originalOriginalColormap = helper->getOriginalColormap();

  // At least one color in the current colormap doesn't have a blue component
  BOOST_CHECK_NE(1, helper->getColormap()->colors[0].blue);
  
  helper->setMonochromeColor(Blue);
  BOOST_CHECK_EQUAL(originalOriginalColormap, helper->getOriginalColormap());

  Colormap::Ptr newColorMap = helper->getColormap();
  BOOST_FOREACH(Color& c, newColorMap->colors)
  {
    BOOST_CHECK_CLOSE(1, c.blue, accuracy);
  }

  Color currentColor = helper->getMonochromeColor();
  BOOST_CHECK_CLOSE(Blue.red, currentColor.red, accuracy);
  BOOST_CHECK_CLOSE(Blue.green, currentColor.green, accuracy);
  BOOST_CHECK_CLOSE(Blue.blue, currentColor.blue, accuracy);
}

BOOST_AUTO_TEST_CASE(inverted_monochrome_colormap_can_have_its_color_set)
{
  ColormapHelper::Ptr helper = MonochromeColormapHelper::createInverted(256);
  Colormap::Ptr originalOriginalColormap = helper->getOriginalColormap();

  // At least one color in the current colormap doesn't have a blue component
  BOOST_CHECK_NE(1, helper->getColormap()->colors.back().blue);
  
  helper->setMonochromeColor(Blue);
  BOOST_CHECK_EQUAL(originalOriginalColormap, helper->getOriginalColormap());

  Colormap::Ptr newColorMap = helper->getColormap();
  BOOST_FOREACH(Color& c, newColorMap->colors)
  {
    BOOST_CHECK_CLOSE(1, c.blue, accuracy);
  }

  Color currentColor = helper->getMonochromeColor();
  BOOST_CHECK_CLOSE(Blue.red, currentColor.red, accuracy);
  BOOST_CHECK_CLOSE(Blue.green, currentColor.green, accuracy);
  BOOST_CHECK_CLOSE(Blue.blue, currentColor.blue, accuracy);
}

BOOST_AUTO_TEST_SUITE_END()
