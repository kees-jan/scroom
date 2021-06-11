#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "../src/ruler.hh"

BOOST_AUTO_TEST_SUITE(Ruler_Tests)

BOOST_AUTO_TEST_CASE(Ruler_creation_signal_handlers,
    * utf::description("Tests that correct signal handlers are registered at creation."))
{
    gtk_init(nullptr, nullptr);
    // Register a new ruler with a dummy drawing area
    GtkWidget* drawingArea = gtk_drawing_area_new ();
    Ruler::Ptr ruler = Ruler::create(Ruler::HORIZONTAL, drawingArea);
    // Check that the appropriate signals are connected

    // Currently, this test case only checks that *a* signal handler is connected
    // for both the "draw" signal and "size-allocate" signal, with a pointer
    // to the ruler as data.
    // It does not check whether drawCallback is connected to "draw" and
    // sizeAllocateCallback to "size-allocate". It probably should, but I can't figure
    // out these weird types.
    auto mask = static_cast<GSignalMatchType>(G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA);
    guint drawID = g_signal_lookup("draw", GTK_TYPE_DRAWING_AREA);
    guint sizeAllocateID = g_signal_lookup("size-allocate", GTK_TYPE_DRAWING_AREA);
    // Check that a signal handler is connected for the "draw" signal, with a pointer to ruler as data
    BOOST_CHECK(g_signal_handler_find(drawingArea, mask, drawID, 0, nullptr, nullptr, ruler.get()) != 0);
    // Check that a signal handler is connected for the "size-allocate" signal, with a pointer to ruler as data
    BOOST_CHECK(g_signal_handler_find(drawingArea, mask, sizeAllocateID, 0, nullptr, nullptr, ruler.get()) != 0);
}

///////////////
// Testing an all-positive range

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_0_to_10_width_1920px,
     * utf::description("Tests the interval for a range [0, 10] for a ruler of width 1920px is 1"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(0, 10, 1920) == 1);
}

///////////////
// Testing an all-positive range

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_236_to_877_width_540px,
     * utf::description("Tests the correct interval is used for range [236, 877] for a ruler of width 540px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(236, 877, 540) == 100);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_236_to_877_width_1920px,
     * utf::description("Tests the correct interval is used for range [236, 877] for a ruler of width 1920px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(236, 877, 1920) == 50);
}

///////////////
// Testing an all-negative range

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_neg791_to_neg312_width_540px,
     * utf::description("Tests the correct interval is used for range [-791, -312] for a ruler of width 540px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(-791, -312, 540) == 100);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_neg791_to_neg312_width_1920px,
     * utf::description("Tests the correct interval is used for range [-791, -312] for a ruler of width 1920px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(-791, -312, 1920) == 25);
}

///////////////
// Testing range with negative and positive part

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_neg513_to_756_width_540px,
     * utf::description("Tests the correct interval is used for range [-513, 756] for a ruler of width 540px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(-513, 756, 540) == 250);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_neg513_to_756_width_1920px,
     * utf::description("Tests the correct interval is used for range [-513, 756] for a ruler of width 1920px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(-513, 756, 1920) == 100);
}

///////////////
// Testing fractional range

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_neg12p56_to27p82_width_540px,
     * utf::description("Tests the correct interval is used for range [-12.56, 27.82] for a ruler of width 540px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(-12.56, 27.82, 540) == 10);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_neg12p56_to27p82_width_1920px,
     * utf::description("Tests the correct interval is used for range [-12.56, 27.82] for a ruler of width 1920px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(-12.56, 27.82, 1920) == 5);
}

///////////////
// Testing range with large numbers

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_negLARGE_to_LARGE_width_540px,
     * utf::description("Tests the correct interval is used for range [-4.2303576974e8, 3.2434878432e8] for a ruler of width 540px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(-4.2303576974e8, 3.2434878432e8, 540) == 250000000);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_negLARGE_to_LARGE_width_1920px,
     * utf::description("Tests the correct interval is used for range [-4.2303576974e8, 3.2434878432e8] for a ruler of width 1920px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(-4.2303576974e8, 3.2434878432e8, 1920) == 50000000);
}

///////////////
// Testing small range of size 1

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_0_to_1_width_540px,
     * utf::description("Tests the correct interval is used for range [0, 1] for a ruler of width 540px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(0, 1, 540) == 1);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_0_to_1_width_1920px,
     * utf::description("Tests the correct interval is used for range [0, 1] for a ruler of width 1920px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(0, 1, 1920) == 1);
}

///////////////
// Testing small range of size < 1

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_0_to_1over10_width_540px,
     * utf::description("Tests the correct interval is used for range [0, 0.1] for a ruler of width 540px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(0, 0.1, 540) == 1);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_0_to_1over10_width_1920px,
     * utf::description("Tests the correct interval is used for range [0, 0.1] for a ruler of width 1920px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(0, 0.1, 1920) == 1);
}

///////////////
// Testing INVALID range lower = upper

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_invalid_range_lower_equals_upper_width_540px,
     * utf::description("Tests that -1 is returned for invalid interval [0,0] for a ruler of width 540px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(0, 0, 540) == -1);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_invalid_range_lower_equals_upper_width_1920px,
     * utf::description("Tests that -1 is returned for invalid interval [0,0] for a ruler of width 1920px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(0, 0, 1920) == -1);
}

///////////////
// Testing INVALID range lower > upper

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_invalid_range_lower_greater_than_upper_width_540px,
     * utf::description("Tests that -1 is returned for invalid interval [0,-100] for a ruler of width 540px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(0, -100, 540) == -1);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalCalculation_invalid_range_lower_greater_than_upper_width_1920px,
     * utf::description("Tests that -1 is returned for invalid interval [0,-100] for a ruler of width 1920px"))
{
    BOOST_CHECK(RulerCalculations::calculateInterval(0, -100, 1920) == -1);
}


///////////////
// Testing scaleToRange()

BOOST_AUTO_TEST_CASE(Ruler_scaleToRange_src_0_to_10__dest_0_100__x_5,
     * utf::description("Tests that scaling 15 from [10, 20] to range [0, 100] gives 50"))
{
    BOOST_CHECK(RulerCalculations::scaleToRange(15, 10, 20, 0, 100) == 50);
}

BOOST_AUTO_TEST_CASE(Ruler_scaleToRange_src_neg28_neg40__dest_0_100__x_neg28,
     * utf::description("Tests that scaling -28 from [-10, -40] to range [0, 100] gives 60"))
{
    BOOST_CHECK(RulerCalculations::scaleToRange(-28, -10, -40, 0, 100) == 60);
}

BOOST_AUTO_TEST_CASE(Ruler_scaleToRange_src_LARGE__dest_LARGE__x_LARGE,
     * utf::description("Tests that scaling 3.532e4 from [-4.230e8, 3.243e8] to range [193, 8.234e5] gives 466198"))
{
    BOOST_CHECK(RulerCalculations::scaleToRange(3.532e4, -4.230e8, 3.243e8, 193, 8.234e5) == 466198);
}

BOOST_AUTO_TEST_CASE(Ruler_scaleToRange_src_SMALL__dest_SMALL__x_SMALL,
     * utf::description("Tests that scaling 3.532e4 from [-4.230e8, 3.243e8] to range [193, 8.234e5] gives 466198"))
{
    BOOST_CHECK(RulerCalculations::scaleToRange(0.23, 0, 1, 0.4, 0.75) == 0.4);
}


///////////////
// Testing intervalPixelSpacing()

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_1_size_540px,
     * utf::description("Tests that an interval of 1 for range [0, 1000] for a ruler of 1920px gives a spacing of 1px"))
{
    BOOST_CHECK(RulerCalculations::intervalPixelSpacing(1, 0, 1000, 540) == 1);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_1_size_1920px,
     * utf::description("Tests that an interval of 1 for range [0, 1000] for a ruler of 1920px gives a spacing of 2px"))
{
    BOOST_CHECK(RulerCalculations::intervalPixelSpacing(1, 0, 1000, 1920) == 2);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_5_size_540px,
     * utf::description("Tests that an interval of 5 for range [0, 1000] for a ruler of 1920px gives a spacing of 1px"))
{
    BOOST_CHECK(RulerCalculations::intervalPixelSpacing(5, 0, 1000, 540) == 3);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_5_size_1920px,
     * utf::description("Tests that an interval of 5 for range [0, 1000] for a ruler of 1920px gives a spacing of 2px"))
{
    BOOST_CHECK(RulerCalculations::intervalPixelSpacing(5, 0, 1000, 1920) == 10);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_25_size_540px,
     * utf::description("Tests that an interval of 25 for range [0, 1000] for a ruler of 1920px gives a spacing of 1px"))
{
    BOOST_CHECK(RulerCalculations::intervalPixelSpacing(25, 0, 1000, 540) == 14);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_25_size_1920px,
     * utf::description("Tests that an interval of 25 for range [0, 1000] for a ruler of 1920px gives a spacing of 2px"))
{
    BOOST_CHECK(RulerCalculations::intervalPixelSpacing(25, 0, 1000, 1920) == 48);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_50000000_size_540px,
     * utf::description("Tests that an interval of 25 for range [0, 1000] for a ruler of 1920px gives a spacing of 1px"))
{
    BOOST_CHECK(RulerCalculations::intervalPixelSpacing(50000000, 0, 1000000000, 540) == 27);
}

BOOST_AUTO_TEST_CASE(Ruler_intervalPixelSpacing_range_0_to_1000_interval_50000000_size_1920px,
     * utf::description("Tests that an interval of 25 for range [0, 1000] for a ruler of 1920px gives a spacing of 2px"))
{
    BOOST_CHECK(RulerCalculations::intervalPixelSpacing(50000000, 0, 1000000000, 1920) == 96);
}

BOOST_AUTO_TEST_SUITE_END()