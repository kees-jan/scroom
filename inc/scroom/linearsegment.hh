/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <ostream>
#include <algorithm>
#include <stdexcept>

#include <boost/operators.hpp>

template<typename T>
class Segment: public boost::addable2<Segment<T>, T>,
    public boost::subtractable2<Segment<T>, T>,
    public boost::multipliable2<Segment<T>, T>,
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

  Segment& moveTo(value_type p)
  {
    start = p;
    return *this;
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
    return getSize () == 0;
  }

  bool operator==(const Segment<value_type>& other) const
  {
    if(isEmpty() != other.isEmpty())
      return false;
    else if(isEmpty())
      return true;
    else
      return (start == other.start) && (size == other.size);
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
  Segment<value_type>& operator&=(const Segment<value_type>& other)
  {
    intersect (other);
    return *this;
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

