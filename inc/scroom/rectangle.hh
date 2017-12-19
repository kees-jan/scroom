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
class Point : public boost::addable<Point<T>>,
              public boost::subtractable<Point<T>>,
              public boost::multipliable2<Point<T>,T>
{
public:
  typedef T value_type;

  Point(value_type x, value_type y)
    : x(x), y(y)
  {}

  bool operator==(const Point<value_type>& other) const
  {
    return x == other.x && y == other.y;
  }

  Point<value_type>& operator+=(const Point<value_type>& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  Point<value_type>& operator-=(const Point<value_type>& other)
  {
    return *this += -other;
  }

  Point<value_type>& operator*=(value_type other)
  {
    x *= other;
    y *= other;
    return *this;
  }

  Point<value_type> operator-() const
  {
    return Point<value_type>(-x, -y);
  }

public:
  value_type x;
  value_type y;
};

template<typename T>
Point<T> make_point(T x, T y)
{
  return Point<T>(x,y);
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Point<T>& p)
{
  return os << '(' << p.x
            << ',' << p.y
            << ')';
}

template<typename T>
class Rectangle : public boost::addable2<Rectangle<T>,Point<T>>,
                  public boost::subtractable2<Rectangle<T>,Point<T>>
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

  void moveTo(Point<value_type> const& other)
  {
    horizontally.moveTo(other.x);
    vertically.moveTo(other.y);
  }

  bool contains(Point<value_type> const& other) const
  {
    return
      horizontally.contains(other.x) &&
      vertically.contains(other.y);
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

  Point<value_type> getTopLeft() const
  {
    return Point<value_type>(horizontally.getStart(), vertically.getStart());
  }

  Point<value_type> getTopRight() const
  {
    return Point<value_type>(horizontally.getEnd(), vertically.getStart());
  }

  Point<value_type> getBottomLeft() const
  {
    return Point<value_type>(horizontally.getStart(), vertically.getEnd());
  }

  Point<value_type> getBottomRight() const
  {
    return Point<value_type>(horizontally.getEnd(), vertically.getEnd());
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

  Rectangle<value_type>& operator+=(Point<value_type> const& other)
  {
    horizontally += other.x;
    vertically += other.y;
    return *this;
  }

  Rectangle<value_type>& operator-=(Point<value_type> const& other)
  {
    return *this += -other;
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
Rectangle<T> make_rect(T x, T y, T width, T height)
{
  return Rectangle<T>(x, y, width, height);
}

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
