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

template<typename T>
class Rectangle
{
public:
  typedef T value_type;
  
  Rectangle()
  {}

  Rectangle(value_type x_, value_type y_, value_type width_, value_type height_)
    : horizontally(x_, width_), vertically(y_, height_)
  {}

  Rectangle(const Segment<value_type>& horizontally_, const Segment<value_type>& vertically_)
    : horizontally(horizontally_), vertically(vertically_)
  {}

  Rectangle(const GdkRectangle& rect)
  : horizontally(rect.x, rect.width), vertically(rect.y, rect.height)
  {}

  operator GdkRectangle() const
  {
    return Scroom::GtkHelpers::createGdkRectangle(getLeftPos(), getTopPos(), getWidth(), getHeight());
  }

  void moveTo(value_type x, value_type y)
  {
    horizontally.moveTo(x);
    vertically.moveTo(y);
  }

  bool containsPos(value_type xVal, value_type yVal) const
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

  void reduceSizeToMultipleOf(value_type size)
  {
    horizontally.reduceSizeToMultipleOf(size);
    vertically.reduceSizeToMultipleOf(size);
  }

  Rectangle intersection(const Rectangle& other) const
  {
    return Rectangle(horizontally.intersection(other.horizontally),
                     vertically.intersection(other.vertically));
  }

  value_type getTopPos() const
  {
    return vertically.getStart();
  }

  value_type getLeftPos() const
  {
    return horizontally.getStart();
  }

  value_type getBottomPos() const
  {
    return vertically.getEnd();
  }

  value_type getRightPos() const
  {
    return horizontally.getEnd();
  }

  value_type getWidth() const
  {
    return horizontally.getSize();
  }

  value_type getHeight() const
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

  const Segment<value_type>& getHorizontally() const
  {
    return horizontally;
  }

  const Segment<value_type>& getVertically() const
  {
    return vertically;
  }

private:
  Segment<value_type> horizontally;
  Segment<value_type> vertically;
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const Rectangle<T>& r)
{
  os << '<' << r.getLeftPos()
     << ',' << r.getTopPos()
     << ',' << r.getWidth()
     << ',' << r.getHeight()
     << '>';

  return os;
}
