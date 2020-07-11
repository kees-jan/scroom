/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <ostream>

#include <boost/operators.hpp>
#include <gdk/gdk.h>

namespace Scroom
{
  namespace Utils
  {
    
    template<typename T>
    class Point : public boost::addable<Point<T>>,
                  public boost::subtractable<Point<T>>,
                  public boost::multipliable2<Point<T>,T>,
                  public boost::dividable2<Point<T>,T>
    {
    public:
      typedef T value_type;

      Point(value_type x_, value_type y_)
        : x(x_), y(y_)
      {}

      template<typename U>
      explicit Point(Point<U> other)
        : x(other.x), y(other.y)
      {}

      explicit Point(GdkPoint& other)
        : x(other.x), y(other.y)
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

      Point<value_type>& operator*=(const Point<value_type> &other)
      {
        x *= other.x;
        y *= other.y;
        return *this;
      }

      Point<value_type>& operator/=(value_type other)
      {
        x /= other;
        y /= other;
        return *this;
      }

      Point<value_type>& operator/=(const Point<value_type> &other)
      {
        x /= other.x;
        y /= other.y;
        return *this;
      }

      Point<value_type> operator-() const
      {
        return Point<value_type>(-x, -y);
      }

      double magnitude() const
      {
        return sqrt(x*x+y*y);
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

    template<typename T, typename U>
    Point<typename std::common_type<T,U>::type> operator-(Point<T> left, Point<U> right)
    {
      typedef typename std::common_type<T,U>::type R;
      
      Point<R> result(left);
      result -= Point<R>(right);
      return result;
    }

    template<typename T, typename U>
    Point<typename std::common_type<T,U>::type> operator+(Point<T> left, Point<U> right)
    {
      typedef typename std::common_type<T,U>::type R;
      
      Point<R> result(left);
      result += Point<R>(right);
      return result;
    }
  }
}
