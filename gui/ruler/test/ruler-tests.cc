#include <cmath>

#include <gtest/gtest.h>

#include "ruler.hh"

TEST(Ruler_Tests, Ruler_creation_horizontal_signal_handlers) // NOLINT
{
  gtk_init(nullptr, nullptr);
  // Register a new ruler with a dummy drawing area
  GtkWidget*       drawingArea = gtk_drawing_area_new();
  Ruler::Ptr const ruler       = Ruler::create(Ruler::HORIZONTAL, drawingArea);
  // Check that the appropriate signals are connected

  // Currently, this test case only checks that *a* signal handler is connected
  // for both the "draw" signal and "size-allocate" signal, with a pointer
  // to the ruler as data.
  // It does not check whether drawCallback is connected to "draw" and
  // sizeAllocateCallback to "size-allocate". It probably should, but I can't figure
  // out these weird types.
  auto        mask           = static_cast<GSignalMatchType>(G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA);
  const guint drawID         = g_signal_lookup("draw", GTK_TYPE_DRAWING_AREA);
  const guint sizeAllocateID = g_signal_lookup("size-allocate", GTK_TYPE_DRAWING_AREA);
  // Check that a signal handler is connected for the "draw" signal, with a pointer to ruler as data
  EXPECT_TRUE(g_signal_handler_find(drawingArea, mask, drawID, 0, nullptr, nullptr, ruler.get()) != 0);
  // Check that a signal handler is connected for the "size-allocate" signal, with a pointer to ruler as data
  EXPECT_TRUE(g_signal_handler_find(drawingArea, mask, sizeAllocateID, 0, nullptr, nullptr, ruler.get()) != 0);
}

TEST(Ruler_Tests, Ruler_creation_vertical_signal_handlers) // NOLINT
{
  gtk_init(nullptr, nullptr);
  // Register a new ruler with a dummy drawing area
  GtkWidget*       drawingArea = gtk_drawing_area_new();
  Ruler::Ptr const ruler       = Ruler::create(Ruler::VERTICAL, drawingArea);
  // Check that the appropriate signals are connected
  auto        mask           = static_cast<GSignalMatchType>(G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA);
  const guint drawID         = g_signal_lookup("draw", GTK_TYPE_DRAWING_AREA);
  const guint sizeAllocateID = g_signal_lookup("size-allocate", GTK_TYPE_DRAWING_AREA);
  // Check that a signal handler is connected for the "draw" signal, with a pointer to ruler as data
  EXPECT_TRUE(g_signal_handler_find(drawingArea, mask, drawID, 0, nullptr, nullptr, ruler.get()) != 0);
  // Check that a signal handler is connected for the "size-allocate" signal, with a pointer to ruler as data
  EXPECT_TRUE(g_signal_handler_find(drawingArea, mask, sizeAllocateID, 0, nullptr, nullptr, ruler.get()) != 0);
}

/**
 * Calculates the minimum pixel width for the ruler such that the interval between major ticks
 * equals expectedInterval.
 * @param lowerLimit Lower limit of the ruler range.
 * @param upperLimit Upper limit of the ruler range.
 * @param expectedInterval The expected interval.
 */
void testCorrectIntervalForMinWidth(double lowerLimit, double upperLimit, int expectedInterval)
{
  const double rangeSize = ceil(upperLimit - lowerLimit);
  // The minimum pixel width of the ruler such the interval can be expectedInterval
  const int minRulerWidth = floor(rangeSize / static_cast<double>(expectedInterval) * RulerCalculations::MIN_SPACE_MAJORTICKS);
  EXPECT_EQ(expectedInterval, RulerCalculations::calculateInterval(lowerLimit, upperLimit, minRulerWidth));
}

TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_1_range_0_to_10) // NOLINT
{
  testCorrectIntervalForMinWidth(0, 10, 1);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_1_range_236_to_877) // NOLINT
{
  testCorrectIntervalForMinWidth(236, 877, 1);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_1_range_158p2_to_687p3) // NOLINT
{
  testCorrectIntervalForMinWidth(158.2, 687.3, 1);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_1_range_neg791_to_neg312) // NOLINT
{
  testCorrectIntervalForMinWidth(-791, -312, 1);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_1_range_neg12p56_to27p82) // NOLINT
{
  testCorrectIntervalForMinWidth(-12.56, 27.82, 1);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_1_range_negLARGE_to_LARGE) // NOLINT
{
  testCorrectIntervalForMinWidth(-4.2303e5, 3.2434e5, 1);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_1_range_0_to_1) // NOLINT
{
  testCorrectIntervalForMinWidth(0, 1, 1);
}

TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_5_range_0_to_10) // NOLINT
{
  testCorrectIntervalForMinWidth(0, 10, 5);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_5_range_236_to_877) // NOLINT
{
  testCorrectIntervalForMinWidth(236, 877, 5);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_5_range_158p2_to_687p3) // NOLINT
{
  testCorrectIntervalForMinWidth(158.2, 687.3, 5);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_5_range_neg791_to_neg312) // NOLINT
{
  testCorrectIntervalForMinWidth(-791, -312, 5);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_5_range_neg12p56_to27p82) // NOLINT
{
  testCorrectIntervalForMinWidth(-12.56, 27.82, 5);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_5_range_negLARGE_to_LARGE) // NOLINT
{
  testCorrectIntervalForMinWidth(-4.2303e5, 3.2434e5, 5);
}
TEST(Ruler_Tests, Ruler_correct_interval_for_minimum_width_interval_5_range_0_to_1) // NOLINT
{
  testCorrectIntervalForMinWidth(0, 1, 5);
}

///////////////
// Testing scaleToRange()

TEST(Ruler_Tests, Ruler_scaleToRange_src_0_to_10_dest_0_100_x_5) // NOLINT
{
  EXPECT_EQ(50, RulerCalculations::scaleToRange(15, 10, 20, 0, 100));
}
TEST(Ruler_Tests, Ruler_scaleToRange_src_neg28_neg40_dest_0_100_x_neg28) // NOLINT
{
  EXPECT_EQ(60, RulerCalculations::scaleToRange(-28, -10, -40, 0, 100));
}
TEST(Ruler_Tests, Ruler_scaleToRange_src_LARGE_dest_LARGE_x_LARGE) // NOLINT
{
  EXPECT_EQ(466198, RulerCalculations::scaleToRange(3.532e4, -4.230e8, 3.243e8, 193, 8.234e5));
}
TEST(Ruler_Tests, Ruler_scaleToRange_src_SMALL_dest_SMALL_x_SMALL) // NOLINT
{
  EXPECT_EQ(0.4, RulerCalculations::scaleToRange(0.23, 0, 1, 0.4, 0.75));
}

///////////////
// Testing intervalPixelSpacing()

TEST(Ruler_Tests, Ruler_intervalPixelSpacing_range_0_to_1000_interval_1_size_540px) // NOLINT
{
  EXPECT_EQ(0, RulerCalculations::intervalPixelSpacing(1, 0, 1000, 540));
}
TEST(Ruler_Tests, Ruler_intervalPixelSpacing_range_0_to_1000_interval_1_size_1920px) // NOLINT
{
  EXPECT_EQ(1, RulerCalculations::intervalPixelSpacing(1, 0, 1000, 1920));
}
TEST(Ruler_Tests, Ruler_intervalPixelSpacing_range_0_to_1000_interval_5_size_540px) // NOLINT
{
  EXPECT_EQ(2, RulerCalculations::intervalPixelSpacing(5, 0, 1000, 540));
}
TEST(Ruler_Tests, Ruler_intervalPixelSpacing_range_0_to_1000_interval_5_size_1920px) // NOLINT
{
  EXPECT_EQ(9, RulerCalculations::intervalPixelSpacing(5, 0, 1000, 1920));
}
TEST(Ruler_Tests, Ruler_intervalPixelSpacing_range_0_to_1000_interval_25_size_540px) // NOLINT
{
  EXPECT_EQ(13, RulerCalculations::intervalPixelSpacing(25, 0, 1000, 540));
}
TEST(Ruler_Tests, Ruler_intervalPixelSpacing_range_0_to_1000_interval_25_size_1920px) // NOLINT
{
  EXPECT_EQ(48, RulerCalculations::intervalPixelSpacing(25, 0, 1000, 1920));
}
TEST(Ruler_Tests, Ruler_intervalPixelSpacing_range_0_to_1000_interval_50000000_size_540px) // NOLINT
{
  EXPECT_EQ(27, RulerCalculations::intervalPixelSpacing(50000000, 0, 1000000000, 540));
}
TEST(Ruler_Tests, Ruler_intervalPixelSpacing_range_0_to_1000_interval_50000000_size_1920px) // NOLINT
{
  EXPECT_EQ(95, RulerCalculations::intervalPixelSpacing(50000000, 0, 1000000000, 1920));
}

///////////////
// Testing firstTick()

TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_0_interval_1) // NOLINT
{
  EXPECT_EQ(0, RulerCalculations::firstTick(0, 1));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_0_interval_25) // NOLINT
{
  EXPECT_EQ(0, RulerCalculations::firstTick(0, 25));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_0_interval_50000) // NOLINT
{
  EXPECT_EQ(0, RulerCalculations::firstTick(0, 50000));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_neg123_interval_1) // NOLINT
{
  EXPECT_EQ(-123, RulerCalculations::firstTick(-123, 1));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_neg123_interval_25) // NOLINT
{
  EXPECT_EQ(-125, RulerCalculations::firstTick(-123, 25));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_neg123_interval_50000) // NOLINT
{
  EXPECT_EQ(-50000, RulerCalculations::firstTick(-123, 50000));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_360_interval_1) // NOLINT
{
  EXPECT_EQ(360, RulerCalculations::firstTick(360, 1));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_360_interval_25) // NOLINT
{
  EXPECT_EQ(350, RulerCalculations::firstTick(360, 25));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_360_interval_50000) // NOLINT
{
  EXPECT_EQ(0, RulerCalculations::firstTick(360, 50000));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_0p1_interval_1) // NOLINT
{
  EXPECT_EQ(0, RulerCalculations::firstTick(0.1, 1));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_0p1_interval_25) // NOLINT
{
  EXPECT_EQ(0, RulerCalculations::firstTick(0.1, 25));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_0p1_interval_50000) // NOLINT
{
  EXPECT_EQ(0, RulerCalculations::firstTick(0.1, 50000));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_neg0p1_interval_1) // NOLINT
{
  EXPECT_EQ(-1, RulerCalculations::firstTick(-0.1, 1));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_neg0p1_interval_25) // NOLINT
{
  EXPECT_EQ(-25, RulerCalculations::firstTick(-0.1, 25));
}
TEST(Ruler_Tests, Ruler_firstTick_lowerLimit_neg0p1_interval_50000) // NOLINT
{
  EXPECT_EQ(-50000, RulerCalculations::firstTick(-0.1, 50000));
}
