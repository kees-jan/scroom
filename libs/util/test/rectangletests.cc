/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gtest/gtest.h>

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
  static const int verticalStart = -1;
  static const int verticalContainedStart = 0;
  static const int verticalContainedWidth = 1;
  static const int verticalSize = 5;

public:
  RectangleHorizontalTestScaffold(int horizontalStart, int horizontalSize)
    : Rectangle(horizontalStart, verticalStart, horizontalSize, verticalSize)
  {
  }

  RectangleHorizontalTestScaffold() = default;

  RectangleHorizontalTestScaffold(const Rectangle<int>& r) // NOLINT(hicpp-explicit-conversions)
    : Rectangle(r)
  {
  }

  [[nodiscard]] int getSize() const { return getWidth(); }

  [[nodiscard]] int getStart() const { return getLeft(); }
  [[nodiscard]] int getEnd() const { return getLeft() + getWidth(); }
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
  static const int horizontalStart = -1;
  static const int horizontalContainedStart = 0;
  static const int horizontalContainedWidth = 1;
  static const int horizontalSize = 5;

public:
  RectangleVerticalTestScaffold(int verticalStart, int verticalSize)
    : Rectangle(horizontalStart, verticalStart, horizontalSize, verticalSize)
  {
  }

  RectangleVerticalTestScaffold() = default;

  RectangleVerticalTestScaffold(const Rectangle<int>& r) // NOLINT(hicpp-explicit-conversions)
    : Rectangle(r)
  {
  }

  [[nodiscard]] int getSize() const { return getHeight(); }
  [[nodiscard]] int getStart() const { return getTop(); }
  [[nodiscard]] int getEnd() const { return getTop() + getHeight(); }
  [[nodiscard]] bool contains(int y) const { return Rectangle::contains(make_point(horizontalContainedStart, y)); }

  [[nodiscard]] bool contains(const RectangleVerticalTestScaffold& other) const { return Rectangle<int>::contains(other); }

  [[nodiscard]] RectangleVerticalTestScaffold intersection(const RectangleVerticalTestScaffold& other) const
  {
    return Rectangle<int>::intersection(other);
  }

  Rectangle moveTo(int y) { return Rectangle::moveTo(make_point(horizontalContainedStart, y)); }
};

using ScaffoldTypes = ::testing::Types<Segment<int>, RectangleHorizontalTestScaffold, RectangleVerticalTestScaffold>;

////////////////////////////////////////////////////////////////////////
// Tests

template <class Scaffold>
void containedSegmentEqualsIntersection(const Scaffold& container, const Scaffold& contained)
{
  if(container.contains(contained))
  {
    EXPECT_EQ(contained, container.intersection(contained));
    EXPECT_EQ(contained, contained.intersection(container));
  }
}

template <class Scaffold>
void intersectsImpliesNonEmptyIntersection(const Scaffold& a, const Scaffold& b)
{
  EXPECT_EQ(a.intersects(b), b.intersects(a));
  EXPECT_EQ(a.intersects(b), !a.intersection(b).isEmpty());
  EXPECT_EQ(b.intersects(a), !b.intersection(a).isEmpty());
  EXPECT_EQ(a.intersection(b), b.intersection(a));
}

template <typename ScaffoldT>
class Rectangle_and_Segment_TypedTests : public ::testing::Test
{
};

TYPED_TEST_SUITE(Rectangle_and_Segment_TypedTests, ScaffoldTypes);

TYPED_TEST(Rectangle_and_Segment_TypedTests, testCreateSegment) // NOLINT
{
  using Scaffold = TypeParam;
  Scaffold s1;
  Scaffold s2(2, 5);
  Scaffold s3;
  Scaffold s4(2, 5);
  Scaffold s5(3, 5);
  Scaffold s6(2, 6);
  Scaffold s7(7, -5);

  EXPECT_TRUE(s1.isEmpty());
  EXPECT_EQ(0, s1.getSize());

  EXPECT_FALSE(s2.isEmpty());
  EXPECT_TRUE(s3.isEmpty());
  EXPECT_FALSE(s4.isEmpty());
  EXPECT_FALSE(s5.isEmpty());
  EXPECT_FALSE(s6.isEmpty());

  EXPECT_EQ(2, s2.getStart());
  EXPECT_EQ(5, s2.getSize());
  EXPECT_EQ(7, s2.getEnd());
  EXPECT_TRUE(s1 == s3);
  EXPECT_TRUE(s2 == s4);
  EXPECT_TRUE(s1 != s2);
  EXPECT_TRUE(s2 != s5);
  EXPECT_TRUE(s2 != s6);

  EXPECT_EQ(2, s7.getStart());
  EXPECT_EQ(5, s7.getSize());
  EXPECT_EQ(7, s7.getEnd());
  EXPECT_EQ(s2, s7);
}

TYPED_TEST(Rectangle_and_Segment_TypedTests, testMoveTo) // NOLINT
{
  using Scaffold = TypeParam;
  Scaffold orig(2, 5);
  Scaffold s = orig.moveTo(5);

  EXPECT_FALSE(s.isEmpty());

  EXPECT_EQ(5, s.getStart());
  EXPECT_EQ(5, s.getSize());
  EXPECT_EQ(10, s.getEnd());
}

TYPED_TEST(Rectangle_and_Segment_TypedTests, testReduceSizeToMultipleOf) // NOLINT
{
  using Scaffold = TypeParam;
  Scaffold s(2, 10);

  s.reduceSizeToMultipleOf(5);
  EXPECT_EQ(2, s.getStart());
  EXPECT_EQ(10, s.getSize());

  s.reduceSizeToMultipleOf(3);
  EXPECT_EQ(2, s.getStart());
  EXPECT_EQ(9, s.getSize());
}

TYPED_TEST(Rectangle_and_Segment_TypedTests, testContainsPoint) // NOLINT
{
  using Scaffold = TypeParam;
  Scaffold s1;
  Scaffold s2(2, 5);

  EXPECT_FALSE(s1.contains(0));
  EXPECT_FALSE(s2.contains(0));
  EXPECT_TRUE(s2.contains(2));
  EXPECT_TRUE(s2.contains(6));
  EXPECT_FALSE(s2.contains(7));
  EXPECT_FALSE(s2.contains(8));
}

TYPED_TEST(Rectangle_and_Segment_TypedTests, testContainsSegment) // NOLINT
{
  using Scaffold = TypeParam;
  Scaffold s1;
  Scaffold s2(-1, 3);
  Scaffold s3(-3, 2);
  Scaffold s4(-2, 2);
  Scaffold s5(-1, 2);
  Scaffold s6(0, 2);
  Scaffold s7(1, 2);
  Scaffold s8(2, 2);

  EXPECT_FALSE(s1.contains(s2));
  containedSegmentEqualsIntersection(s1, s2);
  EXPECT_FALSE(s1.contains(s3));
  containedSegmentEqualsIntersection(s1, s3);
  EXPECT_FALSE(s1.contains(s4));
  containedSegmentEqualsIntersection(s1, s4);
  EXPECT_FALSE(s1.contains(s5));
  containedSegmentEqualsIntersection(s1, s5);
  EXPECT_FALSE(s1.contains(s6));
  containedSegmentEqualsIntersection(s1, s6);
  EXPECT_FALSE(s1.contains(s7));
  containedSegmentEqualsIntersection(s1, s7);
  EXPECT_FALSE(s1.contains(s8));
  containedSegmentEqualsIntersection(s1, s8);

  EXPECT_TRUE(s2.contains(s1));
  containedSegmentEqualsIntersection(s2, s1);
  EXPECT_FALSE(s2.contains(s3));
  containedSegmentEqualsIntersection(s2, s3);
  EXPECT_FALSE(s2.contains(s4));
  containedSegmentEqualsIntersection(s2, s4);
  EXPECT_TRUE(s2.contains(s5));
  containedSegmentEqualsIntersection(s2, s5);
  EXPECT_TRUE(s2.contains(s6));
  containedSegmentEqualsIntersection(s2, s6);
  EXPECT_FALSE(s2.contains(s7));
  containedSegmentEqualsIntersection(s2, s7);
  EXPECT_FALSE(s2.contains(s8));
  containedSegmentEqualsIntersection(s2, s8);
}

TYPED_TEST(Rectangle_and_Segment_TypedTests, testIntersects) // NOLINT
{
  using Scaffold = TypeParam;
  Scaffold s1;
  Scaffold s2(-1, 3);
  Scaffold s3(-3, 2);
  Scaffold s4(-2, 2);
  Scaffold s5(-1, 2);
  Scaffold s6(0, 2);
  Scaffold s7(1, 2);
  Scaffold s8(2, 2);

  EXPECT_FALSE(s1.intersects(s2));
  intersectsImpliesNonEmptyIntersection(s1, s2);
  EXPECT_FALSE(s1.intersects(s3));
  intersectsImpliesNonEmptyIntersection(s1, s3);
  EXPECT_FALSE(s1.intersects(s4));
  intersectsImpliesNonEmptyIntersection(s1, s4);
  EXPECT_FALSE(s1.intersects(s5));
  intersectsImpliesNonEmptyIntersection(s1, s5);
  EXPECT_FALSE(s1.intersects(s6));
  intersectsImpliesNonEmptyIntersection(s1, s6);
  EXPECT_FALSE(s1.intersects(s7));
  intersectsImpliesNonEmptyIntersection(s1, s7);
  EXPECT_FALSE(s1.intersects(s8));
  intersectsImpliesNonEmptyIntersection(s1, s8);

  EXPECT_FALSE(s2.intersects(s1));
  intersectsImpliesNonEmptyIntersection(s2, s1);
  EXPECT_FALSE(s2.intersects(s3));
  intersectsImpliesNonEmptyIntersection(s2, s3);
  EXPECT_TRUE(s2.intersects(s4));
  intersectsImpliesNonEmptyIntersection(s2, s4);
  EXPECT_TRUE(s2.intersects(s5));
  intersectsImpliesNonEmptyIntersection(s2, s5);
  EXPECT_TRUE(s2.intersects(s6));
  intersectsImpliesNonEmptyIntersection(s2, s6);
  EXPECT_TRUE(s2.intersects(s7));
  intersectsImpliesNonEmptyIntersection(s2, s7);
  EXPECT_FALSE(s2.intersects(s8));
  intersectsImpliesNonEmptyIntersection(s2, s8);
}

TYPED_TEST(Rectangle_and_Segment_TypedTests, testIntersection) // NOLINT
{
  using Scaffold = TypeParam;
  Scaffold s1;
  Scaffold s2(-1, 3);
  Scaffold s3(-3, 2);
  Scaffold s4(-2, 2);
  Scaffold s5(-1, 2);
  Scaffold s6(0, 2);
  Scaffold s7(1, 2);
  Scaffold s8(2, 2);

  EXPECT_TRUE(s1.intersection(s2).isEmpty());
  EXPECT_TRUE(s1.intersection(s3).isEmpty());
  EXPECT_TRUE(s1.intersection(s4).isEmpty());
  EXPECT_TRUE(s1.intersection(s5).isEmpty());
  EXPECT_TRUE(s1.intersection(s6).isEmpty());
  EXPECT_TRUE(s1.intersection(s7).isEmpty());
  EXPECT_TRUE(s1.intersection(s8).isEmpty());

  EXPECT_TRUE(s2.intersection(s1).isEmpty());
  EXPECT_TRUE(s2.intersection(s3).isEmpty());
  EXPECT_EQ(Scaffold(-1, 1), s2.intersection(s4));
  EXPECT_EQ(s5, s2.intersection(s5));
  EXPECT_EQ(s6, s2.intersection(s6));
  EXPECT_EQ(Scaffold(1, 1), s2.intersection(s7));
  EXPECT_TRUE(s2.intersection(s8).isEmpty());
}

////////////////////////////////////////////////////////////////////////

TEST(Rectangle_and_Segment_Tests, testRetrievingHorizontallyAndVertically) // NOLINT
{
  const Rectangle<int> r(1, 2, 3, 4);

  EXPECT_EQ(make_segment(1, 3), r.getHorizontally());
  EXPECT_EQ(make_segment(2, 4), r.getVertically());
}

TEST(Rectangle_and_Segment_Tests, testConversionToAndFromGdkRectangle) // NOLINT
{
  const cairo_rectangle_int_t original = Scroom::GtkHelpers::createCairoIntRectangle(1, 2, 3, 4);
  const auto rect = Rectangle<int>(original);
  EXPECT_EQ(make_rect(1, 2, 3, 4), rect);

  const GdkRectangle grect = rect.toGdkRectangle();
  EXPECT_EQ(original, grect);
}

TEST(Rectangle_and_Segment_Tests, testCorners) // NOLINT
{
  const Rectangle<int> rect(1, 2, 3, 4);
  EXPECT_EQ(make_point(1, 2), rect.getTopLeft());
  EXPECT_EQ(make_point(4, 2), rect.getTopRight());
  EXPECT_EQ(make_point(1, 6), rect.getBottomLeft());
  EXPECT_EQ(make_point(4, 6), rect.getBottomRight());
}

TEST(Rectangle_and_Segment_Tests, testMath) // NOLINT
{
  EXPECT_EQ(make_point(4, 6), make_point(1, 2) + make_point(3, 4));
  EXPECT_EQ(make_point(4, 6) - make_point(1, 2), make_point(3, 4));
  EXPECT_EQ(make_point(2, 4), make_point(1, 2) * 2);

  EXPECT_EQ(make_rect(6, 8, 3, 4), make_point(5, 6) + make_rect(1, 2, 3, 4));
  EXPECT_EQ(make_rect(6, 8, 3, 4) - make_point(5, 6), make_rect(1, 2, 3, 4));

  auto result = 0.5 * make_rect(1, 3, 5, 7);
  EXPECT_NEAR(0.5, result.getLeft(), 1e-6);
  EXPECT_NEAR(2.5, result.getWidth(), 1e-6);
  EXPECT_NEAR(1.5, result.getTop(), 1e-6);
  EXPECT_NEAR(3.5, result.getHeight(), 1e-6);
}

TEST(Rectangle_and_Segment_Tests, testCuts) // NOLINT
{
  const Rectangle<int> original(10, 20, 30, 40);

  EXPECT_TRUE(original.leftOf(5).isEmpty());
  EXPECT_EQ(original, original.leftOf(40));
  EXPECT_EQ(Rectangle<int>(10, 20, 10, 40), original.leftOf(20));

  EXPECT_TRUE(original.rightOf(40).isEmpty());
  EXPECT_EQ(original, original.rightOf(5));
  EXPECT_EQ(Rectangle<int>(20, 20, 20, 40), original.rightOf(20));

  EXPECT_TRUE(original.above(5).isEmpty());
  EXPECT_EQ(original, original.above(60));
  EXPECT_EQ(Rectangle<int>(10, 20, 30, 20), original.above(40));

  EXPECT_TRUE(original.below(60).isEmpty());
  EXPECT_EQ(original, original.below(20));
  EXPECT_EQ(Rectangle<int>(10, 40, 30, 20), original.below(40));

  EXPECT_EQ(Rectangle<int>(10, 30, 5, 5), original.leftOf(Rectangle<int>(15, 30, 10, 5)));
  EXPECT_EQ(Rectangle<int>(10, 20, 10, 40), original.above(Rectangle<int>(0, 100, 20, 10)));
}

TEST(Rectangle_and_Segment_Tests, testPlus) // NOLINT
{
  Segment<int> const result = 5 + make_segment(7, 3);
  Segment<int> const expected(12, 3);
  EXPECT_EQ(expected, result);
}

TEST(Rectangle_and_Segment_Tests, testMinus) // NOLINT
{
  Segment<int> const result = make_segment(7, 3) - 10;
  Segment<int> const expected(-3, 3);
  EXPECT_EQ(expected, result);
}

TEST(Rectangle_and_Segment_Tests, testMultiply) // NOLINT
{
  Segment<int> const result = make_segment(7, 3) * 5;
  Segment<int> const expected(35, 15);
  EXPECT_EQ(expected, result);
}

TEST(Rectangle_and_Segment_Tests, testAnd) // NOLINT
{
  Segment<int> const left(1, 4);
  Segment<int> const right(3, 7);
  Segment<int> const result = left & right;
  Segment<int> const expected(3, 2);
  EXPECT_EQ(expected, result);
}
