/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <type_traits>

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include <scroom/colormappable.hh>
#include <scroom/unused.hh>


namespace
{
  const Color Blue(0,0,1);
  const double accuracy = 1e-4;

  struct Data
  {
    ColormapHelper::Ptr helper;
    int expectedColors;

    Data(int expectedColors_, ColormapHelper::Ptr const& helper_)
      : helper(helper_), expectedColors(expectedColors_)
    {}
  };

  std::ostream& operator<<(std::ostream& os, const Data&)
  {
    return os;
  }

  static std::list<Data> helpers = boost::assign::list_of
                          (Data(4, ColormapHelper::create(4)))
                          (Data(2, ColormapHelper::create(Colormap::createDefault(2))))
                          (Data(4, ColormapHelper::createInverted(4)))
                          (Data(256, MonochromeColormapHelper::create(256)))
                          ;
}

BOOST_AUTO_TEST_SUITE(ColormapHelper_Tests)

BOOST_DATA_TEST_CASE(colormaps_equal_and_correct_count, boost::unit_test::data::make(helpers))
{
  Colormap::Ptr originalColormap = sample.helper->getOriginalColormap();
  BOOST_REQUIRE(originalColormap);
  BOOST_CHECK_EQUAL(sample.expectedColors, originalColormap->colors.size());

  Colormap::Ptr colormap = sample.helper->getColormap();
  BOOST_REQUIRE(colormap);
  BOOST_CHECK_EQUAL(sample.expectedColors, colormap->colors.size());
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
  for(Color& c: newColorMap->colors)
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
  for(Color& c: newColorMap->colors)
  {
    BOOST_CHECK_CLOSE(1, c.blue, accuracy);
  }

  Color currentColor = helper->getMonochromeColor();
  BOOST_CHECK_CLOSE(Blue.red, currentColor.red, accuracy);
  BOOST_CHECK_CLOSE(Blue.green, currentColor.green, accuracy);
  BOOST_CHECK_CLOSE(Blue.blue, currentColor.blue, accuracy);
}

BOOST_AUTO_TEST_SUITE_END()
