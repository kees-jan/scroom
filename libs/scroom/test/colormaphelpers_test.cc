/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <scroom/colormappable.hh>

namespace
{
  const Color  Blue(0, 0, 1);
  const double accuracy = 1e-6;

  struct Data
  {
    ColormapHelper::Ptr helper;
    size_t              expectedColors;

    Data(int expectedColors_, ColormapHelper::Ptr helper_)
      : helper(std::move(helper_))
      , expectedColors(expectedColors_)
    {
    }
  };

  const std::vector<Data> helpers = {
    Data(4, ColormapHelper::create(4)),
    Data(2, ColormapHelper::create(Colormap::createDefault(2))),
    Data(4, ColormapHelper::createInverted(4)),
    Data(256, MonochromeColormapHelper::create(256)),
  };

  class ColormapHelper_Data_Tests : public ::testing::TestWithParam<Data>
  {
  };

} // namespace

INSTANTIATE_TEST_SUITE_P(ColormapHelper_Tests, ColormapHelper_Data_Tests, ::testing::ValuesIn(helpers));

TEST_P(ColormapHelper_Data_Tests, colormaps_equal_and_correct_count) // NOLINT
{
  const Data& sample = GetParam();

  Colormap::Ptr const originalColormap = sample.helper->getOriginalColormap();
  ASSERT_TRUE(originalColormap != nullptr);
  EXPECT_EQ(sample.expectedColors, originalColormap->colors.size());

  Colormap::Ptr const colormap = sample.helper->getColormap();
  ASSERT_TRUE(colormap != nullptr);
  EXPECT_EQ(sample.expectedColors, colormap->colors.size());
  EXPECT_EQ(originalColormap, colormap);
}

TEST(ColormapHelper_Tests, regular_colormaps_cant_have_their_colors_set) // NOLINT
{
  ColormapHelper::Ptr const helper = ColormapHelper::create(256);
  EXPECT_THROW(helper->setMonochromeColor(Color(0, 0, 1)), std::runtime_error);
}

TEST(ColormapHelper_Tests, monochrome_colormap_can_have_its_color_set) // NOLINT
{
  ColormapHelper::Ptr const helper                   = MonochromeColormapHelper::create(256);
  Colormap::Ptr const       originalOriginalColormap = helper->getOriginalColormap();

  // At least one color in the current colormap doesn't have a blue component
  EXPECT_NE(1, helper->getColormap()->colors[0].blue);

  helper->setMonochromeColor(Blue);
  EXPECT_EQ(originalOriginalColormap, helper->getOriginalColormap());

  Colormap::Ptr const newColorMap = helper->getColormap();
  for(Color const& c: newColorMap->colors)
  {
    EXPECT_NEAR(1, c.blue, accuracy);
  }

  Color const currentColor = helper->getMonochromeColor();
  EXPECT_NEAR(Blue.red, currentColor.red, accuracy);
  EXPECT_NEAR(Blue.green, currentColor.green, accuracy);
  EXPECT_NEAR(Blue.blue, currentColor.blue, accuracy);
}

TEST(ColormapHelper_Tests, inverted_monochrome_colormap_can_have_its_color_set) // NOLINT
{
  ColormapHelper::Ptr const helper                   = MonochromeColormapHelper::createInverted(256);
  Colormap::Ptr const       originalOriginalColormap = helper->getOriginalColormap();

  // At least one color in the current colormap doesn't have a blue component
  EXPECT_NE(1, helper->getColormap()->colors.back().blue);

  helper->setMonochromeColor(Blue);
  EXPECT_EQ(originalOriginalColormap, helper->getOriginalColormap());

  Colormap::Ptr const newColorMap = helper->getColormap();
  for(Color const& c: newColorMap->colors)
  {
    EXPECT_NEAR(1, c.blue, accuracy);
  }

  Color const currentColor = helper->getMonochromeColor();
  EXPECT_NEAR(Blue.red, currentColor.red, accuracy);
  EXPECT_NEAR(Blue.green, currentColor.green, accuracy);
  EXPECT_NEAR(Blue.blue, currentColor.blue, accuracy);
}
