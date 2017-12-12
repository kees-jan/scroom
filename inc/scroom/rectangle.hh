/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <ostream>
#include <stdexcept>

#include <gdk/gdktypes.h>

#include <scroom/linearsegment.hh>
#include <scroom/gtk-helpers.hh>

class Rectangle
{
public:
  Rectangle()
  {}

  Rectangle(long x_, long y_, long width_, long height_)
    : horizontally(x_, width_), vertically(y_, height_)
  {}

  Rectangle(const Segment& horizontally_, const Segment& vertically_)
    : horizontally(horizontally_), vertically(vertically_)
  {}

  Rectangle(const GdkRectangle& rect)
  : horizontally(rect.x, rect.width), vertically(rect.y, rect.height)
  {}

  operator GdkRectangle() const
  {
    return Scroom::GtkHelpers::createGdkRectangle(getLeftPos(), getTopPos(), getWidth(), getHeight());
  }

  void moveTo(long x, long y)
  {
    horizontally.moveTo(x);
    vertically.moveTo(y);
  }

  bool containsPos(long xVal, long yVal) const
  {
    return
      horizontally.contains(xVal) &&
      vertically.contains(yVal);
  }

  bool contains(const Rectangle& other) const
  {
    return
      horizontally.contains(other.horizontally) &&
      vertically.contains(other.vertically);
  }

  bool intersects(const Rectangle& other) const
  {
    return
      horizontally.intersects(other.horizontally) &&
      vertically.intersects(other.vertically);
  }

  void reduceSizeToMultipleOf(long size)
  {
    horizontally.reduceSizeToMultipleOf(size);
    vertically.reduceSizeToMultipleOf(size);
  }

  Rectangle intersection(const Rectangle& other) const
  {
    return Rectangle(horizontally.intersection(other.horizontally),
                     vertically.intersection(other.vertically));
  }

  long getTopPos() const
  {
    return vertically.getStart();
  }

  long getLeftPos() const
  {
    return horizontally.getStart();
  }

  long getBottomPos() const
  {
    return vertically.getEnd();
  }

  long getRightPos() const
  {
    return horizontally.getEnd();
  }

  long getWidth() const
  {
    return horizontally.getSize();
  }

  long getHeight() const
  {
    return vertically.getSize();
  }

  bool isEmpty() const
  {
    return horizontally.isEmpty() || vertically.isEmpty();
  }

  bool operator==(const Rectangle& other) const
  {
    return
      horizontally == other.horizontally &&
      vertically == other.vertically;
  }

  bool operator!=(const Rectangle& other) const
  {
    return !(*this==other);
  }

  const Segment& getHorizontally() const
  {
    return horizontally;
  }

  const Segment& getVertically() const
  {
    return vertically;
  }

private:
  Segment horizontally;
  Segment vertically;
};

std::ostream& operator<<(std::ostream& os, const Rectangle& r);
