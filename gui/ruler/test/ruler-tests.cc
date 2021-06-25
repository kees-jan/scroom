#define BOOST_TEST_DYN_LINK
#include <cmath>

#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "ruler.hh"


BOOST_AUTO_TEST_SUITE(Ruler_Tests)

BOOST_AUTO_TEST_CASE(Ruler_creation_horizontal_signal_handlers)
{
  gtk_init(nullptr, nullptr);
  // Register a new ruler with a dummy drawing area
  GtkWidget* drawingArea = gtk_drawing_area_new();
  Ruler::Ptr ruler       = Ruler::create(Ruler::HORIZONTAL, drawingArea);
  // Check that the appropriate signals are connected

  // Currently, this test case only checks that *a* signal handler is connected
  // for both the "draw" signal and "size-allocate" signal, with a pointer
  // to the ruler as data.
  // It does not check whether drawCallback is connected to "draw" and
  // sizeAllocateCallback to "size-allocate". It probably should, but I can't figure
  // out these weird types.
  auto  mask           = static_cast<GSignalMatchType>(G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA);
  guint drawID         = g_signal_lookup("draw", GTK_TYPE_DRAWING_AREA);
  guint sizeAllocateID = g_signal_lookup("size-allocate", GTK_TYPE_DRAWING_AREA);
  // Check that a signal handler is connected for the "draw" signal, with a pointer to ruler as data
  BOOST_CHECK(g_signal_handler_find(drawingArea, mask, drawID, 0, nullptr, nullptr, ruler.get()) != 0);
  // Check that a signal handler is connected for the "size-allocate" signal, with a pointer to ruler as data
  BOOST_CHECK(g_signal_handler_find(drawingArea, mask, sizeAllocateID, 0, nullptr, nullptr, ruler.get()) != 0);
}

BOOST_AUTO_TEST_CASE(Ruler_creation_vertical_signal_handlers)
{
  gtk_init(nullptr, nullptr);
  // Register a new ruler with a dummy drawing area
  GtkWidget* drawingArea = gtk_drawing_area_new();
  Ruler::Ptr ruler       = Ruler::create(Ruler::VERTICAL, drawingArea);
  // Check that the appropriate signals are connected
  auto  mask           = static_cast<GSignalMatchType>(G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA);
  guint drawID         = g_signal_lookup("draw", GTK_TYPE_DRAWING_AREA);
  guint sizeAllocateID = g_signal_lookup("size-allocate", GTK_TYPE_DRAWING_AREA);
  // Check that a signal handler is connected for the "draw" signal, with a pointer to ruler as data
  BOOST_CHECK(g_signal_handler_find(drawingArea, mask, drawID, 0, nullptr, nullptr, ruler.get()) != 0);
  // Check that a signal handler is connected for the "size-allocate" signal, with a pointer to ruler as data
  BOOST_CHECK(g_signal_handler_find(drawingArea, mask, sizeAllocateID, 0, nullptr, nullptr, ruler.get()) != 0);
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
  double rangeSize = ceil(upperLimit - lowerLimit);
  // The minimum pixel width of the ruler such the interval can be expectedInterval
  int minRulerWidth = floor(rangeSize / static_cast<double>(expectedInterval) * RulerCalculations::MIN_SPACE_MAJORTICKS);

  // Test that calculateInterval returns the expected interval
  BOOST_CHECK(RulerCalculations::calculateInterval(lowerLimit, upperLimit, minRulerWidth) == expectedInterval);
}

BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_1_range_0_to_10)
{
  testCorrectIntervalForMinWidth(0, 10, 1);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_1_range_236_to_877)
{
  testCorrectIntervalForMinWidth(236, 877, 1);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_1_range_158p2_to_687p3)
{
  testCorrectIntervalForMinWidth(158.2, 687.3, 1);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_1_range_neg791_to_neg312)
{
  testCorrectIntervalForMinWidth(-791, -312, 1);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_1_range_neg12p56_to27p82)
{
  testCorrectIntervalForMinWidth(-12.56, 27.82, 1);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_1_range_negLARGE_to_LARGE)
{
  testCorrectIntervalForMinWidth(-4.2303e5, 3.2434e5, 1);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_1_range_0_to_1)
{
  testCorrectIntervalForMinWidth(0, 1, 1);
}


BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_5_range_0_to_10)
{
  testCorrectIntervalForMinWidth(0, 10, 5);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_5_range_236_to_877)
{
  testCorrectIntervalForMinWidth(236, 877, 5);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_5_range_158p2_to_687p3)
{
  testCorrectIntervalForMinWidth(158.2, 687.3, 5);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_5_range_neg791_to_neg312)
{
  testCorrectIntervalForMinWidth(-791, -312, 5);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_5_range_neg12p56_to27p82)
{
  testCorrectIntervalForMinWidth(-12.56, 27.82, 5);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_5_range_negLARGE_to_LARGE)
{
  testCorrectIntervalForMinWidth(-4.2303e5, 3.2434e5, 5);
}
BOOST_AUTO_TEST_CASE(Ruler_correct_interval_for_minimum_width_interval_5_range_0_to_1)
{
  testCorrectIntervalForMinWidth(0, 1, 5);
}

///////////////
// Testing scaleToRange()

BOOST_AUTO_TEST_CASE(Ruler_scaleToRange_src_0_to_10_dest_0_100_x_5)
{
  BOOST_CHECK(RulerCalculations::scaleToRange(15, 10, 20, 0, 100) == 50);
}

BOOST_AUTO_TEST_CASE(Ruler_scaleToRange_src_neg28_neg40_dest_0_100_x_neg28)
{
  BOOST_CHECK(RulerCalculations::scaleToRange(-28, -10, -40, 0, 100) == 60);
}

BOOST_AUTO_TEST_CASE(Ruler_scaleToRange_src_LARGE_dest_LARGE_x_LARGE)
{
  BOOST_CHECK(RulerCalculations::scaleToRange(3.532e4, -4.230e8, 3.243e8, 193, 8.234e5) == 466198);
}

BOOST_AUTO_TEST_CASE(Ruler_scaleToRange_src_SMALL_dest_SMALL_x_SMALL)
{
  BOOST_CHECK(RulerCalculations::scaleToRange(0.23, 0, 1, 0.4, 0.75) == 0.4);
}


///////////////
// Testing intervalPixelSpacing()

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_1_size_540px)
{
  BOOST_CHECK(RulerCalculations::intervalPixelSpacing(1, 0, 1000, 540) == 0);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_1_size_1920px)
{
  BOOST_CHECK(RulerCalculations::intervalPixelSpacing(1, 0, 1000, 1920) == 1);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_5_size_540px)
{
  BOOST_CHECK(RulerCalculations::intervalPixelSpacing(5, 0, 1000, 540) == 2);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_5_size_1920px)
{
  BOOST_CHECK(RulerCalculations::intervalPixelSpacing(5, 0, 1000, 1920) == 9);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_25_size_540px)
{
  BOOST_CHECK(RulerCalculations::intervalPixelSpacing(25, 0, 1000, 540) == 13);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_25_size_1920px)
{
  BOOST_CHECK(RulerCalculations::intervalPixelSpacing(25, 0, 1000, 1920) == 48);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_50000000_size_540px)
{
  BOOST_CHECK(RulerCalculations::intervalPixelSpacing(50000000, 0, 1000000000, 540) == 27);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_50000000_size_1920px)
{
  BOOST_CHECK(RulerCalculations::intervalPixelSpacing(50000000, 0, 1000000000, 1920) == 95);
}


///////////////
// Testing firstTick()


BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_0_interval_1) { BOOST_CHECK(RulerCalculations::firstTick(0, 1) == 0); }

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_0_interval_25) { BOOST_CHECK(RulerCalculations::firstTick(0, 25) == 0); }

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_0_interval_50000) { BOOST_CHECK(RulerCalculations::firstTick(0, 50000) == 0); }

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_neg123_interval_1) { BOOST_CHECK(RulerCalculations::firstTick(-123, 1) == -123); }

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_neg123_interval_25)
{
  BOOST_CHECK(RulerCalculations::firstTick(-123, 25) == -125);
}

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_neg123_interval_50000)
{
  BOOST_CHECK(RulerCalculations::firstTick(-123, 50000) == -50000);
}

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_360_interval_1) { BOOST_CHECK(RulerCalculations::firstTick(360, 1) == 360); }

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_360_interval_25) { BOOST_CHECK(RulerCalculations::firstTick(360, 25) == 350); }

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_360_interval_50000)
{
  BOOST_CHECK(RulerCalculations::firstTick(360, 50000) == 0);
}

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_0p1_interval_1) { BOOST_CHECK(RulerCalculations::firstTick(0.1, 1) == 0); }

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_0p1_interval_25) { BOOST_CHECK(RulerCalculations::firstTick(0.1, 25) == 0); }

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_0p1_interval_50000)
{
  BOOST_CHECK(RulerCalculations::firstTick(0.1, 50000) == 0);
}

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_neg0p1_interval_1) { BOOST_CHECK(RulerCalculations::firstTick(-0.1, 1) == -1); }

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_neg0p1_interval_25)
{
  BOOST_CHECK(RulerCalculations::firstTick(-0.1, 25) == -25);
}

BOOST_AUTO_TEST_CASE(Ruler_firstTick_lowerLimit_neg0p1_interval_50000)
{
  BOOST_CHECK(RulerCalculations::firstTick(-0.1, 50000) == -50000);
}

BOOST_AUTO_TEST_SUITE_END()