/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cmath>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <ostream>
#include <type_traits>

#include <boost/operators.hpp>

#include <scroom/rounding.hh>

namespace Scroom::Utils
{
  constexpr double epsilon = 1e-6;

  template <typename T>
  bool isZero(T v);

  template <>
  inline bool isZero<int>(int v)
  {
    return v == 0;
  }

  template <>
  inline bool isZero<double>(double v)
  {
    return std::abs(v) < epsilon;
  }

  template <typename T>
  bool areEqual(T a, T b)
  {
    return isZero(a - b);
  }

  template <typename T>
  concept IntegralCoordinate = std::integral<T> && !std::same_as<std::remove_cv_t<T>, bool>;

  template <IntegralCoordinate T>
  class SegmentIterator : public boost::addable2<SegmentIterator<T>, std::ptrdiff_t>
  {
  public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::input_iterator_tag;

    SegmentIterator() = default;
    SegmentIterator(T current, T end)
      : m_current(current)
      , m_end(end)
    {
    }

    T operator*() const { return m_current; }
    SegmentIterator& operator++()
    {
      ++m_current;
      return *this;
    }
    SegmentIterator operator++(int)
    {
      auto previous = *this;
      ++*this;
      return previous;
    }

    /** Advances by a nonnegative offset; reaching or passing the end compares equal to the sentinel. */
    SegmentIterator& operator+=(difference_type offset)
    {
      m_current = static_cast<T>(m_current + offset);
      return *this;
    }

    bool operator==(const SegmentIterator& other) const { return m_current == other.m_current && m_end == other.m_end; }
    bool operator==(std::default_sentinel_t /*sentinel*/) const { return m_current >= m_end; }

  private:
    T m_current{};
    T m_end{};
  };

  template <typename T>
  class Segment
    : public boost::addable2<Segment<T>, T>
    , public boost::subtractable2<Segment<T>, T>
    , public boost::multipliable2<Segment<T>, T>
    , public boost::dividable2<Segment<T>, T>
    , public boost::andable<Segment<T>>
  {
  public:
    using value_type = T;

    Segment()
      : start(0)
      , size(0)
    {
    }

    Segment(value_type start_, value_type size_)
      : start(start_)
      , size(size_)
    {
      normalize();
    }

    /** Implicit conversion from Segment<int> to Segment<T>. If T!=int */
    template <bool T_is_int = std::is_same_v<int, std::remove_cv_t<T>>>
    explicit Segment(std::enable_if_t<!T_is_int, Segment<int> const&> other)
      : start(other.getStart())
      , size(other.getSize())
    {
    }

    [[nodiscard]] Segment moveTo(value_type p) const { return Segment(p, size); }

    [[nodiscard]] bool contains(value_type p) const { return start <= p && p < (start + size); }

    [[nodiscard]] bool contains(const Segment<value_type>& other) const
    {
      return getStart() <= other.getStart() && other.getEnd() <= getEnd();
    }

    [[nodiscard]] bool intersects(const Segment<value_type>& other) const
    {
      return getStart() < other.getEnd() && other.getStart() < getEnd() && !isEmpty() && !other.isEmpty();
    }

    void reduceSizeToMultipleOf(value_type m) { size -= size % m; }

    [[nodiscard]] Segment<value_type> intersection(const Segment<value_type>& other) const
    {
      const value_type newStart = std::max(getStart(), other.getStart());
      const value_type newEnd = std::min(getEnd(), other.getEnd());
      const value_type newSize = newEnd - newStart;

      if(newSize >= 0)
      {
        return Segment<value_type>(newStart, newSize);
      }

      return Segment<value_type>(); // empty segment
    }

    void intersect(const Segment<value_type>& other) { *this = intersection(other); }

    [[nodiscard]] Segment<value_type> before(value_type v) const
    {
      if(v < start)
      {
        return Segment<value_type>();
      }
      if(v < start + size)
      {
        return Segment<value_type>(start, v - start);
      }
      return *this;
    }

    [[nodiscard]] Segment<value_type> after(value_type v) const
    {
      if(v > start + size)
      {
        return Segment<value_type>();
      }
      if(v > start)
      {
        return Segment<value_type>(v, start + size - v);
      }
      return *this;
    }

    /** Returns an iterator retaining a snapshot of the segment bounds. */
    [[nodiscard]] auto begin() const
      requires IntegralCoordinate<T>
    {
      return SegmentIterator<T>(getStart(), getEnd());
    }

    [[nodiscard]] std::default_sentinel_t end() const
      requires IntegralCoordinate<T>
    {
      return {};
    }

    [[nodiscard]] value_type getStart() const { return start; }

    [[nodiscard]] value_type getEnd() const { return start + size; }

    [[nodiscard]] value_type getSize() const { return size; }

    [[nodiscard]] bool isEmpty() const { return isZero(getSize()); }

    bool operator==(const Segment<value_type>& other) const
    {
      if(isEmpty() != other.isEmpty())
      {
        return false;
      }

      if(isEmpty())
      {
        return true;
      }

      return areEqual(start, other.start) && areEqual(size, other.size);
    }

    bool operator!=(const Segment<value_type>& other) const { return !(*this == other); }

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
      intersect(other);
      return *this;
    }
    void setStart(value_type n)
    {
      const auto end = getEnd();
      start = n;
      setEnd(end);
    }
    void setSize(value_type n) { size = n; }
    void setEnd(value_type n)
    {
      size = n - start;
      normalize();
    }

    template <typename U>
    [[nodiscard]] Segment<U> to() const
    {
      return {
        static_cast<U>(start),
        static_cast<U>(size),
      };
    }

  private:
    void normalize()
    {
      if(size < 0)
      {
        start += size;
        size = -size;
      }
    }


    value_type start;
    value_type size;
  };

  template <typename T>
  Segment<T> make_segment(T start, T size)
  {
    return Segment<T>(start, size);
  }

  template <typename T>
  Segment<T> make_segment_from_start_end(T start, T end)
  {
    return Segment<T>(start, end - start);
  }

  template <typename T>
  std::ostream& operator<<(std::ostream& os, const Segment<T>& s)
  {
    return os << '(' << s.getStart() << ',' << s.getSize() << ')';
  }

  inline Segment<double> roundOutward(Segment<double> s)
  {
    return make_segment_from_start_end(round_down_to_multiple_of(s.getStart(), 1.0), round_up_to_multiple_of(s.getEnd(), 1.0));
  }

  template <typename T>
  T center(Segment<T> s)
  {
    return s.getStart() + s.getSize() / 2;
  }

} // namespace Scroom::Utils
