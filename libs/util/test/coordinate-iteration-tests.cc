/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <ranges>
#include <vector>

#include <gmock/gmock.h>

#include <scroom/rectangle.hh>

using namespace Scroom::Utils;
using testing::ElementsAre;

static_assert(std::ranges::forward_range<Segment<int>>);
static_assert(std::ranges::forward_range<const Segment<unsigned>>);
static_assert(std::ranges::forward_range<Rectangle<int>>);
static_assert(std::ranges::forward_range<const Rectangle<unsigned>>);
static_assert(!std::ranges::range<Segment<double>>);
static_assert(!std::ranges::range<Rectangle<double>>);
static_assert(!std::ranges::range<Segment<bool>>);
static_assert(!std::ranges::range<Rectangle<bool>>);

TEST(SegmentIteration, visits_each_coordinate_excluding_the_end)
{
  const Segment segment(-2, 3);
  std::vector<int> coordinates;
  for(const auto coordinate: segment)
  {
    coordinates.push_back(coordinate);
  }
  EXPECT_THAT(coordinates, ElementsAre(-2, -1, 0));
}

TEST(SegmentIteration, empty_segment_starts_finished)
{
  const Segment empty(7, 0);
  EXPECT_EQ(empty.begin(), empty.end());
}

TEST(SegmentIteration, copies_advance_independently)
{
  const Segment segment(-2, 3);
  auto iterator = segment.begin();
  const auto previous = iterator++;
  EXPECT_EQ(-2, *previous);
  EXPECT_EQ(-1, *iterator);
  EXPECT_NE(previous, iterator);
  EXPECT_EQ(segment.begin(), previous);
}

TEST(SegmentIteration, zero_offset_preserves_position)
{
  const Segment segment(-2, 3);
  auto iterator = segment.begin();
  iterator += 0;
  EXPECT_EQ(segment.begin(), iterator);
}

TEST(SegmentIteration, addition_reaches_the_last_coordinate)
{
  const Segment segment(-2, 3);
  auto iterator = segment.begin();
  const int offsetToLast = segment.getSize() - 1;
  iterator += offsetToLast;
  EXPECT_EQ(0, *iterator);
  EXPECT_EQ(iterator, segment.begin() + offsetToLast);
  EXPECT_EQ(iterator, offsetToLast + segment.begin());
  EXPECT_EQ(segment.end(), ++iterator);
}

TEST(SegmentIteration, exact_and_overshooting_jumps_both_compare_equal_to_the_sentinel)
{
  const Segment segment(-2, 3);
  const auto exactEnd = segment.begin() + segment.getSize();
  const auto pastEnd = segment.begin() + (segment.getSize() + 5);
  EXPECT_EQ(segment.end(), exactEnd);
  EXPECT_EQ(segment.end(), pastEnd);
}

TEST(SegmentIteration, unsigned_coordinates_support_advancement_and_overshooting)
{
  const Segment unsignedSegment(4U, 2U);
  auto iterator = unsignedSegment.begin();
  iterator += 1;
  EXPECT_EQ(5U, *iterator);
  iterator += 3;
  EXPECT_EQ(unsignedSegment.end(), iterator);
}

TEST(SegmentIteration, iterator_retains_its_bounds_after_the_segment_changes)
{
  Segment mutableSegment(4, 2);
  auto iterator = mutableSegment.begin();
  mutableSegment = Segment(20, 10);
  EXPECT_EQ(4, *iterator++);
  EXPECT_EQ(5, *iterator++);
  EXPECT_EQ(std::default_sentinel, iterator);
}

TEST(RectangleIteration, visits_columns_then_rows_and_supports_structured_bindings)
{
  const Rectangle rectangle(-1, 4, 3, 2);
  std::vector<Point<int>> points;
  for(auto [x, y]: rectangle)
  {
    points.emplace_back(x, y);
  }
  EXPECT_THAT(points, ElementsAre(Point(-1, 4), Point(0, 4), Point(1, 4), Point(-1, 5), Point(0, 5), Point(1, 5)));
}

TEST(RectangleIteration, either_zero_dimension_starts_finished)
{
  const Rectangle noColumns(-1, 4, 0, 2);
  const Rectangle noRows(-1, 4, 3, 0);
  EXPECT_EQ(noColumns.begin(), noColumns.end());
  EXPECT_EQ(noRows.begin(), noRows.end());
}

TEST(RectangleIteration, a_single_column_advances_directly_to_the_next_row)
{
  const Rectangle column(2, -1, 1, 2);
  auto iterator = column.begin();
  EXPECT_EQ(Point(2, -1), *iterator++);
  EXPECT_EQ(Point(2, 0), *iterator++);
  EXPECT_EQ(column.end(), iterator);
}

TEST(RectangleIteration, copies_advance_independently)
{
  const Rectangle rectangle(-1, 4, 3, 2);
  auto iterator = rectangle.begin();
  const auto previous = iterator++;
  EXPECT_EQ(rectangle.getTopLeft(), *previous);
  EXPECT_EQ(Point(0, 4), *iterator);
  EXPECT_NE(previous, iterator);
  EXPECT_EQ(rectangle.begin(), previous);
}

TEST(RectangleIteration, zero_offset_preserves_position)
{
  const Rectangle rectangle(-1, 4, 3, 2);
  auto iterator = rectangle.begin();
  iterator += 0;
  EXPECT_EQ(rectangle.begin(), iterator);
}

TEST(RectangleIteration, jumps_cross_rows_from_the_current_column)
{
  const Rectangle rectangle(-1, 4, 3, 2);
  auto iterator = rectangle.begin();
  ++iterator;
  const int oneRowAndOneColumn = rectangle.getWidth() + 1;
  const auto destination = iterator + oneRowAndOneColumn;
  EXPECT_EQ(Point(1, 5), *destination);
  EXPECT_EQ(destination, oneRowAndOneColumn + iterator);
  iterator += oneRowAndOneColumn;
  EXPECT_EQ(destination, iterator);
  EXPECT_EQ(rectangle.end(), ++iterator);
}

TEST(RectangleIteration, exact_and_overshooting_jumps_both_compare_equal_to_the_sentinel)
{
  const Rectangle rectangle(-1, 4, 3, 2);
  const int pointCount = rectangle.getWidth() * rectangle.getHeight();
  const auto exactEnd = rectangle.begin() + pointCount;
  const auto pastEnd = rectangle.begin() + (pointCount + 5);
  EXPECT_EQ(rectangle.end(), exactEnd);
  EXPECT_EQ(rectangle.end(), pastEnd);
}

TEST(RectangleIteration, unsigned_coordinates_support_row_crossings_and_overshooting)
{
  const Rectangle unsignedRectangle(4U, 6U, 2U, 2U);
  auto iterator = unsignedRectangle.begin();
  iterator += unsignedRectangle.getWidth();
  EXPECT_EQ(Point(4U, 7U), *iterator);
  iterator += unsignedRectangle.getWidth() + 1;
  EXPECT_EQ(unsignedRectangle.end(), iterator);
}

TEST(RectangleIteration, iterator_retains_its_bounds_after_the_rectangle_changes)
{
  Rectangle mutableRectangle(4, 6, 1, 1);
  auto iterator = mutableRectangle.begin();
  mutableRectangle = Rectangle(20, 30, 5, 10);
  EXPECT_EQ(Point(4, 6), *iterator++);
  EXPECT_EQ(std::default_sentinel, iterator);
}
