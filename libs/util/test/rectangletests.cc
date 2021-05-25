/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include <scroom/gtk-helpers.hh>
#include <scroom/linearsegment.hh>
#include <scroom/rectangle.hh>

using Scroom::Utils::make_point;
using Scroom::Utils::make_rect;
using Scroom::Utils::make_segment;
using Scroom::Utils::Point;
using Scroom::Utils::Rectangle;
using Scroom::Utils::Segment;

////////////////////////////////////////////////////////////////////////
// For testing Rectangles horizontally

class RectangleHorizontalTestScaffold : public Rectangle<int>
{
private:
  static const int verticalStart          = -1;
  static const int verticalContainedStart = 0;
  static const int verticalContainedWidth = 1;
  static const int verticalSize           = 5;

public:
  RectangleHorizontalTestScaffold(int horizontalStart, int horizontalSize)
    : Rectangle(horizontalStart, verticalStart, horizontalSize, verticalSize)
  {}

  RectangleHorizontalTestScaffold() = default;

  RectangleHorizontalTestScaffold(const Rectangle<int>& r)
    : Rectangle(r)
  {}

  [[nodiscard]] int getSize() const { return getWidth(); }

  [[nodiscard]] int  getStart() const { return getLeft(); }
  [[nodiscard]] int  getEnd() const { return getLeft() + getWidth(); }
  [[nodiscard]] bool contains(int x) const { return Rectangle::contains(make_point(x, verticalContainedStart)); }

  [[nodiscard]] bool contains(const RectangleHorizontalTestScaffold& other) const { return Rectangle<int>::contains(other); }

  [[nodiscard]] RectangleHorizontalTestScaffold intersection(const RectangleHorizontalTestScaffold& other) const
  {
    return Rectangle<int>::intersection(other);
  }

  Rectangle moveTo(int x) { return Rectangle<int>::moveTo(make_point(x, verticalContainedStart)); }
};

////////////////////////////////////////////////////////////////////////
// For testing Rectangles vertically

class RectangleVerticalTestScaffold : public Rectangle<int>
{
private:
  static const int horizontalStart          = -1;
  static const int horizontalContainedStart = 0;
  static const int horizontalContainedWidth = 1;
  static const int horizontalSize           = 5;

public:
  RectangleVerticalTestScaffold(int verticalStart, int verticalSize)
    : Rectangle(horizontalStart, verticalStart, horizontalSize, verticalSize)
  {}

  RectangleVerticalTestScaffold() = default;

  RectangleVerticalTestScaffold(const Rectangle<int>& r)
    : Rectangle(r)
  {}

  [[nodiscard]] int  getSize() const { return getHeight(); }
  [[nodiscard]] int  getStart() const { return getTop(); }
  [[nodiscard]] int  getEnd() const { return getTop() + getHeight(); }
  [[nodiscard]] bool contains(int y) const { return Rectangle::contains(make_point(horizontalContainedStart, y)); }

  [[nodiscard]] bool contains(const RectangleVerticalTestScaffold& other) const { return Rectangle<int>::contains(other); }

  [[nodiscard]] RectangleVerticalTestScaffold intersection(const RectangleVerticalTestScaffold& other) const
  {
    return Rectangle<int>::intersection(other);
  }

  Rectangle moveTo(int y) { return Rectangle::moveTo(make_point(horizontalContainedStart, y)); }
};

using test_types = boost::mpl::list<Segment<int>, RectangleHorizontalTestScaffold, RectangleVerticalTestScaffold>;

////////////////////////////////////////////////////////////////////////
// Tests

BOOST_AUTO_TEST_SUITE(Rectangle_and_Segment_Tests)

template <class Scaffold>
void containedSegmentEqualsIntersection(const Scaffold& container, const Scaffold& contained)
{
  if(container.contains(contained))
  {
    BOOST_CHECK_EQUAL(contained, container.intersection(contained));
    BOOST_CHECK_EQUAL(contained, contained.intersection(container));
  }
}

template <class Scaffold>
void intersectsImpliesNonEmptyIntersection(const Scaffold& a, const Scaffold& b)
{
  BOOST_CHECK_EQUAL(a.intersects(b), b.intersects(a));
  BOOST_CHECK_EQUAL(a.intersects(b), !a.intersection(b).isEmpty());
  BOOST_CHECK_EQUAL(b.intersects(a), !b.intersection(a).isEmpty());
  BOOST_CHECK_EQUAL(a.intersection(b), b.intersection(a));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testCreateSegment, Scaffold, test_types)
{
  Scaffold s1;
  Scaffold s2(2, 5);
  Scaffold s3;
  Scaffold s4(2, 5);
  Scaffold s5(3, 5);
  Scaffold s6(2, 6);
  Scaffold s7(7, -5);

  BOOST_CHECK(s1.isEmpty());
  BOOST_CHECK_EQUAL(0, s1.getSize());

  BOOST_CHECK(!s2.isEmpty());
  BOOST_CHECK(s3.isEmpty());
  BOOST_CHECK(!s4.isEmpty());
  BOOST_CHECK(!s5.isEmpty());
  BOOST_CHECK(!s6.isEmpty());

  BOOST_CHECK_EQUAL(2, s2.getStart());
  BOOST_CHECK_EQUAL(5, s2.getSize());
  BOOST_CHECK_EQUAL(7, s2.getEnd());
  BOOST_CHECK(s1 == s3);
  BOOST_CHECK(s2 == s4);
  BOOST_CHECK(s1 != s2);
  BOOST_CHECK(s2 != s5);
  BOOST_CHECK(s2 != s6);

  BOOST_CHECK_EQUAL(2, s7.getStart());
  BOOST_CHECK_EQUAL(5, s7.getSize());
  BOOST_CHECK_EQUAL(7, s7.getEnd());
  BOOST_CHECK_EQUAL(s2, s7);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testMoveTo, Scaffold, test_types)
{
  Scaffold orig(2, 5);
  Scaffold s = orig.moveTo(5);

  BOOST_CHECK(!s.isEmpty());

  BOOST_CHECK_EQUAL(5, s.getStart());
  BOOST_CHECK_EQUAL(5, s.getSize());
  BOOST_CHECK_EQUAL(10, s.getEnd());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testReduceSizeToMultipleOf, Scaffold, test_types)
{
  Scaffold s(2, 10);

  s.reduceSizeToMultipleOf(5);
  BOOST_CHECK_EQUAL(2, s.getStart());
  BOOST_CHECK_EQUAL(10, s.getSize());

  s.reduceSizeToMultipleOf(3);
  BOOST_CHECK_EQUAL(2, s.getStart());
  BOOST_CHECK_EQUAL(9, s.getSize());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testContainsPoint, Scaffold, test_types)
{
  Scaffold s1;
  Scaffold s2(2, 5);

  BOOST_CHECK(!s1.contains(0));
  BOOST_CHECK(!s2.contains(0));
  BOOST_CHECK(s2.contains(2));
  BOOST_CHECK(s2.contains(6));
  BOOST_CHECK(!s2.contains(7));
  BOOST_CHECK(!s2.contains(8));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testContainsSegment, Scaffold, test_types)
{
  Scaffold s1;
  Scaffold s2(-1, 3);
  Scaffold s3(-3, 2);
  Scaffold s4(-2, 2);
  Scaffold s5(-1, 2);
  Scaffold s6(0, 2);
  Scaffold s7(1, 2);
  Scaffold s8(2, 2);

  BOOST_CHECK(!s1.contains(s2));
  containedSegmentEqualsIntersection(s1, s2);
  BOOST_CHECK(!s1.contains(s3));
  containedSegmentEqualsIntersection(s1, s3);
  BOOST_CHECK(!s1.contains(s4));
  containedSegmentEqualsIntersection(s1, s4);
  BOOST_CHECK(!s1.contains(s5));
  containedSegmentEqualsIntersection(s1, s5);
  BOOST_CHECK(!s1.contains(s6));
  containedSegmentEqualsIntersection(s1, s6);
  BOOST_CHECK(!s1.contains(s7));
  containedSegmentEqualsIntersection(s1, s7);
  BOOST_CHECK(!s1.contains(s8));
  containedSegmentEqualsIntersection(s1, s8);

  BOOST_CHECK(s2.contains(s1));
  containedSegmentEqualsIntersection(s2, s1);
  BOOST_CHECK(!s2.contains(s3));
  containedSegmentEqualsIntersection(s2, s3);
  BOOST_CHECK(!s2.contains(s4));
  containedSegmentEqualsIntersection(s2, s4);
  BOOST_CHECK(s2.contains(s5));
  containedSegmentEqualsIntersection(s2, s5);
  BOOST_CHECK(s2.contains(s6));
  containedSegmentEqualsIntersection(s2, s6);
  BOOST_CHECK(!s2.contains(s7));
  containedSegmentEqualsIntersection(s2, s7);
  BOOST_CHECK(!s2.contains(s8));
  containedSegmentEqualsIntersection(s2, s8);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testIntersects, Scaffold, test_types)
{
  Scaffold s1;
  Scaffold s2(-1, 3);
  Scaffold s3(-3, 2);
  Scaffold s4(-2, 2);
  Scaffold s5(-1, 2);
  Scaffold s6(0, 2);
  Scaffold s7(1, 2);
  Scaffold s8(2, 2);

  BOOST_CHECK(!s1.intersects(s2));
  intersectsImpliesNonEmptyIntersection(s1, s2);
  BOOST_CHECK(!s1.intersects(s3));
  intersectsImpliesNonEmptyIntersection(s1, s3);
  BOOST_CHECK(!s1.intersects(s4));
  intersectsImpliesNonEmptyIntersection(s1, s4);
  BOOST_CHECK(!s1.intersects(s5));
  intersectsImpliesNonEmptyIntersection(s1, s5);
  BOOST_CHECK(!s1.intersects(s6));
  intersectsImpliesNonEmptyIntersection(s1, s6);
  BOOST_CHECK(!s1.intersects(s7));
  intersectsImpliesNonEmptyIntersection(s1, s7);
  BOOST_CHECK(!s1.intersects(s8));
  intersectsImpliesNonEmptyIntersection(s1, s8);

  BOOST_CHECK(!s2.intersects(s1));
  intersectsImpliesNonEmptyIntersection(s2, s1);
  BOOST_CHECK(!s2.intersects(s3));
  intersectsImpliesNonEmptyIntersection(s2, s3);
  BOOST_CHECK(s2.intersects(s4));
  intersectsImpliesNonEmptyIntersection(s2, s4);
  BOOST_CHECK(s2.intersects(s5));
  intersectsImpliesNonEmptyIntersection(s2, s5);
  BOOST_CHECK(s2.intersects(s6));
  intersectsImpliesNonEmptyIntersection(s2, s6);
  BOOST_CHECK(s2.intersects(s7));
  intersectsImpliesNonEmptyIntersection(s2, s7);
  BOOST_CHECK(!s2.intersects(s8));
  intersectsImpliesNonEmptyIntersection(s2, s8);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testIntersection, Scaffold, test_types)
{
  Scaffold s1;
  Scaffold s2(-1, 3);
  Scaffold s3(-3, 2);
  Scaffold s4(-2, 2);
  Scaffold s5(-1, 2);
  Scaffold s6(0, 2);
  Scaffold s7(1, 2);
  Scaffold s8(2, 2);

  BOOST_CHECK(s1.intersection(s2).isEmpty());
  BOOST_CHECK(s1.intersection(s3).isEmpty());
  BOOST_CHECK(s1.intersection(s4).isEmpty());
  BOOST_CHECK(s1.intersection(s5).isEmpty());
  BOOST_CHECK(s1.intersection(s6).isEmpty());
  BOOST_CHECK(s1.intersection(s7).isEmpty());
  BOOST_CHECK(s1.intersection(s8).isEmpty());

  BOOST_CHECK(s2.intersection(s1).isEmpty());
  BOOST_CHECK(s2.intersection(s3).isEmpty());
  BOOST_CHECK_EQUAL(Scaffold(-1, 1), s2.intersection(s4));
  BOOST_CHECK_EQUAL(s5, s2.intersection(s5));
  BOOST_CHECK_EQUAL(s6, s2.intersection(s6));
  BOOST_CHECK_EQUAL(Scaffold(1, 1), s2.intersection(s7));
  BOOST_CHECK(s2.intersection(s8).isEmpty());
}

////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(testRetrievingHorizontallyAndVertically)
{
  const Rectangle<int> r(1, 2, 3, 4);

  BOOST_CHECK_EQUAL(make_segment(1, 3), r.getHorizontally());
  BOOST_CHECK_EQUAL(make_segment(2, 4), r.getVertically());
}

BOOST_AUTO_TEST_CASE(testConversionToAndFromGdkRectangle)
{
  const cairo_rectangle_int_t original = Scroom::GtkHelpers::createGdkRectangle(1, 2, 3, 4);
  const Rectangle<int>        rect     = original;
  BOOST_CHECK_EQUAL(make_rect(1, 2, 3, 4), rect);

  const GdkRectangle grect = rect.toGdkRectangle();
  BOOST_CHECK_EQUAL(original, grect);
}

BOOST_AUTO_TEST_CASE(testCorners)
{
  const Rectangle<int> rect(1, 2, 3, 4);
  BOOST_CHECK_EQUAL(make_point(1, 2), rect.getTopLeft());
  BOOST_CHECK_EQUAL(make_point(4, 2), rect.getTopRight());
  BOOST_CHECK_EQUAL(make_point(1, 6), rect.getBottomLeft());
  BOOST_CHECK_EQUAL(make_point(4, 6), rect.getBottomRight());
}

BOOST_AUTO_TEST_CASE(testMath)
{
  BOOST_CHECK_EQUAL(make_point(4, 6), make_point(1, 2) + make_point(3, 4));
  BOOST_CHECK_EQUAL(make_point(4, 6) - make_point(1, 2), make_point(3, 4));
  BOOST_CHECK_EQUAL(make_point(2, 4), make_point(1, 2) * 2);

  BOOST_CHECK_EQUAL(make_rect(6, 8, 3, 4), make_point(5, 6) + make_rect(1, 2, 3, 4));
  BOOST_CHECK_EQUAL(make_rect(6, 8, 3, 4) - make_point(5, 6), make_rect(1, 2, 3, 4));

  auto result = 0.5 * make_rect(1, 3, 5, 7);
  BOOST_CHECK_CLOSE(0.5, result.getLeft(), 1e-6);
  BOOST_CHECK_CLOSE(2.5, result.getWidth(), 1e-6);
  BOOST_CHECK_CLOSE(1.5, result.getTop(), 1e-6);
  BOOST_CHECK_CLOSE(3.5, result.getHeight(), 1e-6);
}

BOOST_AUTO_TEST_CASE(testCuts)
{
  const Rectangle<int> original(10, 20, 30, 40);

  BOOST_CHECK(original.leftOf(5).isEmpty());
  BOOST_CHECK_EQUAL(original, original.leftOf(40));
  BOOST_CHECK_EQUAL(Rectangle<int>(10, 20, 10, 40), original.leftOf(20));

  BOOST_CHECK(original.rightOf(40).isEmpty());
  BOOST_CHECK_EQUAL(original, original.rightOf(5));
  BOOST_CHECK_EQUAL(Rectangle<int>(20, 20, 20, 40), original.rightOf(20));

  BOOST_CHECK(original.above(5).isEmpty());
  BOOST_CHECK_EQUAL(original, original.above(60));
  BOOST_CHECK_EQUAL(Rectangle<int>(10, 20, 30, 20), original.above(40));

  BOOST_CHECK(original.below(60).isEmpty());
  BOOST_CHECK_EQUAL(original, original.below(20));
  BOOST_CHECK_EQUAL(Rectangle<int>(10, 40, 30, 20), original.below(40));

  BOOST_CHECK_EQUAL(Rectangle<int>(10, 30, 5, 5), original.leftOf(Rectangle<int>(15, 30, 10, 5)));
  BOOST_CHECK_EQUAL(Rectangle<int>(10, 20, 10, 40), original.above(Rectangle<int>(0, 100, 20, 10)));
}

BOOST_AUTO_TEST_CASE(testPlus)
{
  Segment<int> result = 5 + make_segment(7, 3);
  Segment<int> expected(12, 3);
  BOOST_CHECK_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(testMinus)
{
  Segment<int> result = make_segment(7, 3) - 10;
  Segment<int> expected(-3, 3);
  BOOST_CHECK_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(testMultiply)
{
  Segment<int> result = make_segment(7, 3) * 5;
  Segment<int> expected(35, 15);
  BOOST_CHECK_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(testAnd)
{
  Segment<int> left(1, 4);
  Segment<int> right(3, 7);
  Segment<int> result = left & right;
  Segment<int> expected(3, 2);
  BOOST_CHECK_EQUAL(expected, result);
}

BOOST_AUTO_TEST_SUITE_END()
