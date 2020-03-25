/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <ostream>

#include <boost/operators.hpp>

#include <scroom/gtk-helpers.hh>
#include <scroom/linearsegment.hh>
#include <scroom/point.hh>

namespace Scroom
{
  namespace Utils
  {
  
    template<typename T>
    class Rectangle : public boost::addable2<Rectangle<T>,Point<T>>,
                      public boost::subtractable2<Rectangle<T>,Point<T>>,
                      public boost::multipliable2<Rectangle<T>,T>,
                      public boost::multipliable2<Rectangle<T>,Point<T>>,
                      public boost::dividable2<Rectangle<T>,T>,
                      public boost::dividable2<Rectangle<T>,Point<T>>
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

      Rectangle(const Rectangle<int>& rect)
        : horizontally(rect.getHorizontally()), vertically(rect.getVertically())
      {}

      GdkRectangle toGdkRectangle() const
      {
        return Scroom::GtkHelpers::createGdkRectangle(getLeft(), getTop(), getWidth(), getHeight());
      }

      Rectangle<int> toIntRectangle() const
      {
        return Rectangle<int>(getLeft(), getTop(), getWidth(), getHeight());
      }

      Rectangle moveTo(Point<value_type> const& other) const
      {
        return moveTo(other.x, other.y);
      }

      Rectangle moveTo(value_type const& x, value_type const& y) const
      {
        return Rectangle(horizontally.moveTo(x), vertically.moveTo(y));
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

      template<typename U>
      Rectangle<typename std::common_type<T,U>::type> intersection(const Rectangle<U>& other) const
      {
        typedef typename std::common_type<T,U>::type R;
        
        return Rectangle<R>(*this).intersection(Rectangle<R>(other));
      }

      value_type getTop() const
      {
        return vertically.getStart();
      }

      value_type getLeft() const
      {
        return horizontally.getStart();
      }

      value_type getBottom() const
      {
        return vertically.getEnd();
      }

      value_type getRight() const
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

      value_type x() const
      {
        return getLeft();
      }

      value_type y() const
      {
        return getTop();
      }

      value_type width() const
      {
        return getWidth();
      }

      value_type height() const
      {
        return getHeight();
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

      Rectangle<value_type> leftOf(value_type v) const
      {
        return Rectangle<value_type>(horizontally.before(v), vertically);
      }

      Rectangle<value_type> rightOf(value_type v) const
      {
        return Rectangle<value_type>(horizontally.after(v), vertically);
      }

      Rectangle<value_type> above(value_type v) const
      {
        return Rectangle<value_type>(horizontally, vertically.before(v));
      }

      Rectangle<value_type> below(value_type v) const
      {
        return Rectangle<value_type>(horizontally, vertically.after(v));
      }

      Rectangle<value_type> leftOf(Rectangle<value_type> const& r) const
      {
        if(r.isEmpty())
          return r;

        return Rectangle<value_type>(horizontally.before(r.horizontally.getStart()),
                                     vertically.intersection(r.vertically));
      }

      Rectangle<value_type> rightOf(Rectangle<value_type> const& r) const
      {
        if(r.isEmpty())
          return r;

        return Rectangle<value_type>(horizontally.after(r.horizontally.getEnd()),
                                     vertically.intersection(r.vertically));
      }

      Rectangle<value_type> above(Rectangle<value_type> const& r) const
      {
        if(r.isEmpty())
          return r;

        return Rectangle<value_type>(horizontally.intersection(r.horizontally),
                                     vertically.before(r.vertically.getStart()));
      }

      Rectangle<value_type> below(Rectangle<value_type> const& r) const
      {
        if(r.isEmpty())
          return r;

        return Rectangle<value_type>(horizontally.intersection(r.horizontally),
                                     vertically.after(r.vertically.getEnd()));
      }

      bool operator==(const Rectangle& other) const
      {
        if(isEmpty() != other.isEmpty())
          return false;
        else if(isEmpty())
          return true;
        else
          return horizontally == other.horizontally && vertically == other.vertically;
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

      Rectangle<value_type>& operator*=(value_type other)
      {
        horizontally *= other;
        vertically *= other;
        return *this;
      }

      Rectangle<value_type>& operator*=(Point<value_type> const& other)
      {
        horizontally *= other.x;
        vertically *= other.y;
        return *this;
      }

      Rectangle<value_type>& operator/=(value_type other)
      {
        horizontally /= other;
        vertically /= other;
        return *this;
      }

      Rectangle<value_type>& operator/=(Point<value_type> const& other)
      {
        horizontally /= other.x;
        vertically /= other.y;
        return *this;
      }

      const Segment<value_type>& getHorizontally() const
      {
        return horizontally;
      }

      const Segment<value_type>& getVertically() const
      {
        return vertically;
      }

      void setSize(Point<value_type> const& s)
      {
        horizontally.setSize(s.x);
        vertically.setSize(s.y);
      }

      Point<value_type> getSize() const
      {
        return Point<value_type>(horizontally.getSize(), vertically.getSize());
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
      os << '<' << r.getLeft()
         << ',' << r.getTop()
         << ',' << r.getWidth()
         << ',' << r.getHeight()
         << '>';

      return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const GdkRectangle& r)
    {
      return os << Rectangle<double>(r);
    }

    template<typename T, typename U>
    Rectangle<typename std::common_type<T,U>::type> operator*(Rectangle<T> left, U right)
    {
      Rectangle<typename std::common_type<T,U>::type> result(left);
      result *= static_cast<typename std::common_type<T,U>::type>(right);
      return result;
    }

    template<typename T, typename U>
    Rectangle<typename std::common_type<T,U>::type> operator*(T left, Rectangle<U> right)
    {
      return right*left;
    }

    inline Rectangle<double> operator*(GdkRectangle const& r, Point<double> const& p)
    {
      return Rectangle<double>(r) * p;
    }

    inline Rectangle<double> operator*(Point<double> const& p, GdkRectangle const& r)
    {
      return Rectangle<double>(r) * p;
    }

    template<typename T, typename U>
    Rectangle<typename std::common_type<T,U>::type> operator-(Rectangle<T> left, Point<U> right)
    {
      typedef typename std::common_type<T,U>::type R;

      return Rectangle<R>(left) - Point<R>(right);
    }

    inline Rectangle<int> roundOutward(Rectangle<double> r)
    {
      int x = floor(r.getLeft());
      int y = floor(r.getTop());
      int width = ceil(r.getRight()) - x;
      int height = ceil(r.getBottom()) - y;

      return Rectangle<int>(x,y,width,height);
    }


  }
}
