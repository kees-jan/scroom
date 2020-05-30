/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <ostream>
#include <cmath>
#include <type_traits>

#include <boost/operators.hpp>

namespace Scroom
{
  namespace Utils
  {
    
    template<typename T>
    bool isZero(T v);

    template<>
    inline bool isZero<int>(int v)
    {
      return v==0;
    }

    template<>
    inline bool isZero<double>(double v)
    {
      return std::abs(v) < 1e-6;
    }

    template<typename T>
    bool areEqual(T a, T b)
    {
      return isZero(a-b);
    }

    template<typename T>
    class Segment: public boost::addable2<Segment<T>, T>,
                   public boost::subtractable2<Segment<T>, T>,
                   public boost::multipliable2<Segment<T>, T>,
                   public boost::dividable2<Segment<T>, T>,
                   public boost::andable<Segment<T>>
    {
    public:
      typedef T value_type;

      Segment()
        : start (0), size (0)
      {}

      Segment(value_type start_, value_type size_)
        : start (start_), size (size_)
      {
        if (size < 0)
        {
          start += size;
          size = -size;
        }
      }

      /** Implicit conversion from Segment<int> to Segment<T>. If T!=int */
      template<bool T_is_int = std::is_same<int, typename std::remove_cv<T>::type>::value>
      Segment(typename std::enable_if<!T_is_int,Segment<int> const&>::type other)
        : start(other.getStart()), size(other.getSize())
      {
      }

      Segment moveTo(value_type p) const
      {
        return Segment(p, size);
      }

      bool contains(value_type p) const
      {
        return start <= p && p < (start + size);
      }

      bool contains(const Segment<value_type>& other) const
      {
        return getStart () <= other.getStart () && other.getEnd () <= getEnd ();
      }

      bool intersects(const Segment<value_type>& other) const
      {
        return getStart () < other.getEnd () && other.getStart () < getEnd () && !isEmpty () && !other.isEmpty ();
      }

      void reduceSizeToMultipleOf(value_type m)
      {
        size -= size % m;
      }

      Segment<value_type> intersection(const Segment<value_type>& other) const
      {
        const value_type newStart = std::max (getStart (), other.getStart ());
        const value_type newEnd = std::min (getEnd (), other.getEnd ());
        const value_type newSize = newEnd - newStart;

        if (newSize >= 0)
          return Segment<value_type> (newStart, newSize);
        else
          return Segment<value_type> (); // empty segment
      }

      void intersect(const Segment<value_type>& other)
      {
        *this = intersection (other);
      }

      Segment<value_type> before(value_type v) const
      {
        if(v < start)
          return Segment<value_type>();
        if(v < start+size)
          return Segment<value_type>(start, v-start);
        return *this;
      }

      Segment<value_type> after(value_type v) const
      {
        if(v > start+size)
          return Segment<value_type>();
        if(v > start)
          return Segment<value_type>(v, start+size-v);
        return *this;
      }

      value_type getStart() const
      {
        return start;
      }

      value_type getEnd() const
      {
        return start + size;
      }

      value_type getSize() const
      {
        return size;
      }

      bool isEmpty() const
      {
        return isZero(getSize());
      }

      bool operator==(const Segment<value_type>& other) const
      {
        if(isEmpty() != other.isEmpty())
          return false;
        else if(isEmpty())
          return true;
        else
          return areEqual(start, other.start) && areEqual(size, other.size);
      }

      bool operator!=(const Segment<value_type>& other) const
      {
        return !(*this == other);
      }

      Segment<value_type>& operator+=(value_type n)
      {
        start += n;
        return *this;
      }
      Segment<value_type>& operator-=(value_type n)
      {
        start -= n;
        return *this;
      }
      Segment<value_type>& operator*=(value_type n)
      {
        start *= n;
        size *= n;
        return *this;
      }
      Segment<value_type>& operator/=(value_type n)
      {
        start /= n;
        size /= n;
        return *this;
      }
      Segment<value_type>& operator&=(const Segment<value_type>& other)
      {
        intersect (other);
        return *this;
      }
      void setSize(value_type n)
      {
        size = n;
      }

    private:
      value_type start;
      value_type size;
    };

    template<typename T>
    Segment<T> make_segment(T start, T size)
    {
      return Segment<T>(start, size);
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const Segment<T>& s)
    {
      return os << '(' << s.getStart()
                << ',' << s.getSize()
                << ')';
    }

  }
}
