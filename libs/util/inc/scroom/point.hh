/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cmath>
#include <ostream>

#include <boost/operators.hpp>

#include <gdk/gdk.h>

#include <scroom/rounding.hh>

namespace Scroom::Utils
{

  template <typename T>
  class Point
    : public boost::addable<Point<T>>
    , public boost::subtractable<Point<T>>
    , public boost::multipliable2<Point<T>, T>
    , public boost::dividable2<Point<T>, T>
  {
  public:
    using value_type = T;

    Point() = default;

    explicit Point(value_type xy)
      : Point(xy, xy)
    {
    }

    Point(value_type x_, value_type y_)
      : x(x_)
      , y(y_)
    {
    }

    template <typename U>
    explicit Point(Point<U> other)
      : x(other.x)
      , y(other.y)
    {
    }

    explicit Point(const GdkPoint& other)
      : x(other.x)
      , y(other.y)
    {
    }

    bool operator==(const Point<value_type>& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Point<value_type>& other) const { return !(*this == other); }

    Point<value_type>& operator+=(const Point<value_type>& other)
    {
      x += other.x;
      y += other.y;
      return *this;
    }

    Point<value_type>& operator-=(const Point<value_type>& other) { return *this += -other; }

    Point<value_type>& operator*=(value_type other)
    {
      x *= other;
      y *= other;
      return *this;
    }

    Point<value_type>& operator*=(const Point<value_type>& other)
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

    Point<value_type>& operator/=(const Point<value_type>& other)
    {
      x /= other.x;
      y /= other.y;
      return *this;
    }

    Point<value_type> operator-() const { return Point<value_type>(-x, -y); }

    [[nodiscard]] double magnitude() const { return sqrt(x * x + y * y); }

    template <typename U>
    [[nodiscard]] Point<U> to() const
    {
      return {static_cast<U>(x), static_cast<U>(y)};
    }

    [[nodiscard]] GdkPoint toGdkPoint() const { return {static_cast<int>(x), static_cast<int>(y)}; }

  public:
    value_type x{0};
    value_type y{0};
  };

  template <typename T>
  Point<T> make_point(T x, T y)
  {
    return Point<T>(x, y);
  }

  template <typename T>
  Point<T> make_point(T c)
  {
    return Point<T>(c, c);
  }

  template <typename T>
  std::ostream& operator<<(std::ostream& os, const Point<T>& p)
  {
    return os << '(' << p.x << ',' << p.y << ')';
  }

  template <typename T, typename U>
  Point<typename std::common_type<T, U>::type> operator-(Point<T> left, Point<U> right)
  {
    using R = typename std::common_type<T, U>::type;

    Point<R> result(left);
    result -= Point<R>(right);
    return result;
  }

  template <typename T, typename U>
  Point<typename std::common_type<T, U>::type> operator+(Point<T> left, Point<U> right)
  {
    using R = typename std::common_type<T, U>::type;

    Point<R> result(left);
    result += Point<R>(right);
    return result;
  }

  template <typename T, typename U>
  Point<typename std::common_type<T, U>::type> operator*(Point<T> left, Point<U> right)
  {
    using R = typename std::common_type<T, U>::type;

    Point<R> result(left);
    result *= Point<R>(right);
    return result;
  }

  template <typename T, typename U>
  Point<typename std::common_type<T, U>::type> operator/(Point<T> left, Point<U> right)
  {
    using R = typename std::common_type<T, U>::type;

    Point<R> result(left);
    result /= Point<R>(right);
    return result;
  }

  template <typename T, typename U>
  Point<typename std::common_type<T, U>::type> operator/(T left, Point<U> right)
  {
    return make_point(left, left) / right;
  }

  template <typename T, typename U>
  Point<typename std::common_type<T, U>::type> operator/(Point<T> left, U right)
  {
    return left / make_point(right, right);
  }

  template <typename T>
  Point<T> rounded_divide_by(Point<T> value, T factor)
  {
    using ::rounded_divide_by;
    return {rounded_divide_by(value.x, factor), rounded_divide_by(value.y, factor)};
  }

  template <typename T>
  Point<T> ceiled_divide_by(Point<T> value, T factor)
  {
    using ::ceiled_divide_by;
    return {ceiled_divide_by(value.x, factor), ceiled_divide_by(value.y, factor)};
  }

  template <typename T>
  Point<T> floored_divide_by(Point<T> value, T factor)
  {
    using ::floored_divide_by;
    return {floored_divide_by(value.x, factor), floored_divide_by(value.y, factor)};
  }

  template <typename T>
  Point<T> rounded_divide_by(Point<T> value, Point<T> factor)
  {
    using ::rounded_divide_by;
    return {rounded_divide_by(value.x, factor.x), rounded_divide_by(value.y, factor.y)};
  }

  template <typename T>
  Point<T> ceiled_divide_by(Point<T> value, Point<T> factor)
  {
    using ::ceiled_divide_by;
    return {ceiled_divide_by(value.x, factor.x), ceiled_divide_by(value.y, factor.y)};
  }

  template <typename T>
  Point<T> floored_divide_by(Point<T> value, Point<T> factor)
  {
    using ::floored_divide_by;
    return {floored_divide_by(value.x, factor.x), floored_divide_by(value.y, factor.y)};
  }

  template <typename T>
  Point<T> ceil(Point<T> p)
  {
    using std::ceil;
    return {ceil(p.x), ceil(p.y)};
  }

} // namespace Scroom::Utils
