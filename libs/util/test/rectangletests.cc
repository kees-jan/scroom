/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/linearsegment.hh>
#include <scroom/rectangle.hh>

#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

std::ostream& operator<<(std::ostream& os, Segment const segment)
{
  return os << '(' << segment.getStart() << ", " << segment.getSize() << ')';
}

////////////////////////////////////////////////////////////////////////
// For testing Rectangles horizontally

class RectangleHorizontalTestScaffold : public Rectangle
{
private:
  static const long verticalStart = -1;
  static const long verticalContainedStart = 0;
  static const long verticalContainedWidth = 1;
  static const long verticalSize = 5;
  
public:
  RectangleHorizontalTestScaffold(long horizontalStart, long horizontalSize)
    : Rectangle(horizontalStart, verticalStart, horizontalSize, verticalSize)
  {}

  RectangleHorizontalTestScaffold()
    : Rectangle()
  {}

  RectangleHorizontalTestScaffold(const Rectangle& r)
    : Rectangle(r)
  {}

  long getSize() const { return getWidth(); }

  long getStart() const { return getLeftPos(); }
  long getEnd() const { return getLeftPos()+getWidth(); }
  bool contains(int x) const { return containsPos(x, verticalContainedStart); }
  
  bool contains(const RectangleHorizontalTestScaffold& other) const
  { return Rectangle::contains(other); }
  
  RectangleHorizontalTestScaffold intersection(const RectangleHorizontalTestScaffold& other) const
  { return Rectangle::intersection(other); }
  
  void moveTo(long x) { return Rectangle::moveTo(x, verticalContainedStart); }
};


////////////////////////////////////////////////////////////////////////
// For testing Rectangles vertically

class RectangleVerticalTestScaffold : public Rectangle
{
private:
  static const long horizontalStart = -1;
  static const long horizontalContainedStart = 0;
  static const long horizontalContainedWidth = 1;
  static const long horizontalSize = 5;
  
public:
  RectangleVerticalTestScaffold(long verticalStart, long verticalSize)
    : Rectangle(horizontalStart, verticalStart, horizontalSize, verticalSize)
  {}

  RectangleVerticalTestScaffold()
    : Rectangle()
  {}

  RectangleVerticalTestScaffold(const Rectangle& r)
    : Rectangle(r)
  {}

  long getSize() const { return getHeight(); }
  long getStart() const { return getTopPos(); }
  long getEnd() const { return getTopPos()+getHeight(); }
  bool contains(int y) const { return containsPos(horizontalContainedStart, y); }
  
  bool contains(const RectangleVerticalTestScaffold& other) const
  { return Rectangle::contains(other); }
  
  RectangleVerticalTestScaffold intersection(const RectangleVerticalTestScaffold& other) const
  { return Rectangle::intersection(other); }
  
  void moveTo(long y) { return Rectangle::moveTo(horizontalContainedStart, y); }
};

typedef boost::mpl::list<Segment, RectangleHorizontalTestScaffold, RectangleVerticalTestScaffold> test_types;

////////////////////////////////////////////////////////////////////////
// Tests

BOOST_AUTO_TEST_SUITE(Rectangle_and_Segment_Tests)

template<class Scaffold>
void containedSegmentEqualsIntersection(const Scaffold& container, const Scaffold& contained)
{
  if(container.contains(contained))
  {
    BOOST_CHECK_EQUAL(contained, container.intersection(contained));
    BOOST_CHECK_EQUAL(contained, contained.intersection(container));
  }
}

template<class Scaffold>
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
  Scaffold s2(2,5);
  Scaffold s3;
  Scaffold s4(2,5);
  Scaffold s5(3,5);
  Scaffold s6(2,6);
  Scaffold s7(7,-5);

  BOOST_CHECK(s1.isEmpty());
  BOOST_CHECK_EQUAL(long(0), s1.getSize());
  
  BOOST_CHECK(!s2.isEmpty());
  BOOST_CHECK(s3.isEmpty());
  BOOST_CHECK(!s4.isEmpty());
  BOOST_CHECK(!s5.isEmpty());
  BOOST_CHECK(!s6.isEmpty());
  
  BOOST_CHECK_EQUAL(long(2), s2.getStart());
  BOOST_CHECK_EQUAL(long(5), s2.getSize());
  BOOST_CHECK_EQUAL(long(7), s2.getEnd());
  BOOST_CHECK(s1==s3);
  BOOST_CHECK(s2==s4);
  BOOST_CHECK(s1!=s2);
  BOOST_CHECK(s2!=s5);
  BOOST_CHECK(s2!=s6);

  BOOST_CHECK_EQUAL(long(2), s7.getStart());
  BOOST_CHECK_EQUAL(long(5), s7.getSize());
  BOOST_CHECK_EQUAL(long(7), s7.getEnd());
  BOOST_CHECK_EQUAL(s2, s7);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testMoveTo, Scaffold, test_types)
{
  Scaffold s(2,5);

  s.moveTo(5);

  BOOST_CHECK(!s.isEmpty());
  
  BOOST_CHECK_EQUAL(long(5), s.getStart());
  BOOST_CHECK_EQUAL(long(5), s.getSize());
  BOOST_CHECK_EQUAL(long(10), s.getEnd());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testReduceSizeToMultipleOf, Scaffold, test_types)
{
  Scaffold s(2,10);

  s.reduceSizeToMultipleOf(5);
  BOOST_CHECK_EQUAL(long(2), s.getStart());
  BOOST_CHECK_EQUAL(long(10), s.getSize());

  s.reduceSizeToMultipleOf(3);
  BOOST_CHECK_EQUAL(long(2), s.getStart());
  BOOST_CHECK_EQUAL(long(9), s.getSize());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testContainsPoint, Scaffold, test_types)
{
  Scaffold s1;
  Scaffold s2(2,5);

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
  Scaffold s7(1,2);
  Scaffold s8(2,2);

  BOOST_CHECK(!s1.contains(s2));
  containedSegmentEqualsIntersection(s1,s2);
  BOOST_CHECK(!s1.contains(s3));
  containedSegmentEqualsIntersection(s1,s3);
  BOOST_CHECK(!s1.contains(s4));
  containedSegmentEqualsIntersection(s1,s4);
  BOOST_CHECK(!s1.contains(s5));
  containedSegmentEqualsIntersection(s1,s5);
  BOOST_CHECK(!s1.contains(s6));
  containedSegmentEqualsIntersection(s1,s6);
  BOOST_CHECK(!s1.contains(s7));
  containedSegmentEqualsIntersection(s1,s7);
  BOOST_CHECK(!s1.contains(s8));
  containedSegmentEqualsIntersection(s1,s8);

  BOOST_CHECK(s2.contains(s1));
  containedSegmentEqualsIntersection(s2,s1);
  BOOST_CHECK(!s2.contains(s3));
  containedSegmentEqualsIntersection(s2,s3);
  BOOST_CHECK(!s2.contains(s4));
  containedSegmentEqualsIntersection(s2,s4);
  BOOST_CHECK(s2.contains(s5));
  containedSegmentEqualsIntersection(s2,s5);
  BOOST_CHECK(s2.contains(s6));
  containedSegmentEqualsIntersection(s2,s6);
  BOOST_CHECK(!s2.contains(s7));
  containedSegmentEqualsIntersection(s2,s7);
  BOOST_CHECK(!s2.contains(s8));
  containedSegmentEqualsIntersection(s2,s8);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testIntersects, Scaffold, test_types)
{
  Scaffold s1;
  Scaffold s2(-1, 3);
  Scaffold s3(-3, 2);
  Scaffold s4(-2, 2);
  Scaffold s5(-1, 2);
  Scaffold s6(0, 2);
  Scaffold s7(1,2);
  Scaffold s8(2,2);

  BOOST_CHECK(!s1.intersects(s2));
  intersectsImpliesNonEmptyIntersection(s1,s2);
  BOOST_CHECK(!s1.intersects(s3));
  intersectsImpliesNonEmptyIntersection(s1,s3);
  BOOST_CHECK(!s1.intersects(s4));
  intersectsImpliesNonEmptyIntersection(s1,s4);
  BOOST_CHECK(!s1.intersects(s5));
  intersectsImpliesNonEmptyIntersection(s1,s5);
  BOOST_CHECK(!s1.intersects(s6));
  intersectsImpliesNonEmptyIntersection(s1,s6);
  BOOST_CHECK(!s1.intersects(s7));
  intersectsImpliesNonEmptyIntersection(s1,s7);
  BOOST_CHECK(!s1.intersects(s8));
  intersectsImpliesNonEmptyIntersection(s1,s8);

  BOOST_CHECK(!s2.intersects(s1));
  intersectsImpliesNonEmptyIntersection(s2,s1);
  BOOST_CHECK(!s2.intersects(s3));
  intersectsImpliesNonEmptyIntersection(s2,s3);
  BOOST_CHECK(s2.intersects(s4));
  intersectsImpliesNonEmptyIntersection(s2,s4);
  BOOST_CHECK(s2.intersects(s5));
  intersectsImpliesNonEmptyIntersection(s2,s5);
  BOOST_CHECK(s2.intersects(s6));
  intersectsImpliesNonEmptyIntersection(s2,s6);
  BOOST_CHECK(s2.intersects(s7));
  intersectsImpliesNonEmptyIntersection(s2,s7);
  BOOST_CHECK(!s2.intersects(s8));
  intersectsImpliesNonEmptyIntersection(s2,s8);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(testIntersection, Scaffold, test_types)
{
  Scaffold s1;
  Scaffold s2(-1, 3);
  Scaffold s3(-3, 2);
  Scaffold s4(-2, 2);
  Scaffold s5(-1, 2);
  Scaffold s6(0, 2);
  Scaffold s7(1,2);
  Scaffold s8(2,2);

  BOOST_CHECK(s1.intersection(s2).isEmpty());
  BOOST_CHECK(s1.intersection(s3).isEmpty());
  BOOST_CHECK(s1.intersection(s4).isEmpty());
  BOOST_CHECK(s1.intersection(s5).isEmpty());
  BOOST_CHECK(s1.intersection(s6).isEmpty());
  BOOST_CHECK(s1.intersection(s7).isEmpty());
  BOOST_CHECK(s1.intersection(s8).isEmpty());

  BOOST_CHECK(s2.intersection(s1).isEmpty());
  BOOST_CHECK(s2.intersection(s3).isEmpty());
  BOOST_CHECK_EQUAL(Scaffold(-1,1), s2.intersection(s4));
  BOOST_CHECK_EQUAL(s5, s2.intersection(s5));
  BOOST_CHECK_EQUAL(s6, s2.intersection(s6));
  BOOST_CHECK_EQUAL(Scaffold(1,1), s2.intersection(s7));
  BOOST_CHECK(s2.intersection(s8).isEmpty());
}

////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(testRetrievingHorizontallyAndVertically)
{
  const Rectangle r(1,2,3,4);

  BOOST_CHECK_EQUAL(Segment(1,3), r.getHorizontally());
  BOOST_CHECK_EQUAL(Segment(2,4), r.getVertically());
}

////////////////////////////////////////////////////////////////////////
// Tests

BOOST_AUTO_TEST_CASE(testPlus)
{
  Segment result = 5 + Segment(7,3);
  Segment expected(12,3);
  BOOST_CHECK_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(testMinus)
{
  Segment result =  Segment(7,3)-10;
  Segment expected(-3,3);
  BOOST_CHECK_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(testMultiply)
{
  Segment result = Segment(7,3)*5;
  Segment expected(35,15);
  BOOST_CHECK_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(testAnd)
{
  Segment left(1,4);
  Segment right(3,7);
  Segment result = left & right;
  Segment expected(3,2);
  BOOST_CHECK_EQUAL(expected, result);
}

BOOST_AUTO_TEST_SUITE_END()

  
