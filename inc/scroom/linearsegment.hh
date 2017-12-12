/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <ostream>
#include <algorithm>
#include <stdexcept>

#include <boost/operators.hpp>

class Segment: public boost::addable2<Segment, int>,
    public boost::subtractable2<Segment, int>,
    public boost::multipliable2<Segment, int>,
    public boost::andable<Segment>
{
public:
  Segment()
      : start (0), size (0)
  {}

  Segment(long start_, long size_)
      : start (start_), size (size_)
  {
    if (size < 0)
    {
      start += size;
      size = -size;
    }
  }

  Segment& moveTo(long p)
  {
    start = p;
    return *this;
  }

  bool contains(long p) const
  {
    return start <= p && p < (start + size);
  }

  bool contains(const Segment& other) const
  {
    return getStart () <= other.getStart () && other.getEnd () <= getEnd ();
  }

  bool intersects(const Segment& other) const
  {
    return getStart () < other.getEnd () && other.getStart () < getEnd () && !isEmpty () && !other.isEmpty ();
  }

  void reduceSizeToMultipleOf(long m)
  {
    size -= size % m;
  }

  Segment intersection(const Segment& other) const
  {
    const long newStart = std::max (getStart (), other.getStart ());
    const long newEnd = std::min (getEnd (), other.getEnd ());
    const long newSize = newEnd - newStart;

    if (newSize >= 0)
      return Segment (newStart, newSize);
    else
      return Segment (); // empty segment
  }

  void intersect(const Segment& other)
  {
    *this = intersection (other);
  }

  long getStart() const
  {
    return start;
  }

  long getEnd() const
  {
    return start + size;
  }

  long getSize() const
  {
    return size;
  }

  bool isEmpty() const
  {
    return getSize () == 0;
  }

  bool operator==(const Segment& other) const
  {
    return (start == other.start) && (size == other.size);
  }

  bool operator!=(const Segment& other) const
  {
    return !(*this == other);
  }

  Segment& operator+=(int n)
  {
    start += n;
    return *this;
  }
  Segment& operator-=(int n)
  {
    start -= n;
    return *this;
  }
  Segment& operator*=(int n)
  {
    start *= n;
    size *= n;
    return *this;
  }
  Segment& operator&=(const Segment& other)
  {
    intersect (other);
    return *this;
  }

private:
  long start;
  long size;
};

std::ostream& operator<<(std::ostream& os, const Segment& s);
