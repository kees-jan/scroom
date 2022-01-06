/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>

#include <boost/function.hpp>

namespace Scroom
{
  namespace Detail
  {
    namespace ThreadPool
    {
      class FunctionAdditor
      {
      private:
        std::list<boost::function<void()>> functions;

      public:
        void addBefore(boost::function<void()> const& fn);
        void addAfter(boost::function<void()> const& fn);

        FunctionAdditor& operator+(boost::function<void()> const& fn);
        FunctionAdditor& operator+=(boost::function<void()> const& fn);
        void             operator()();
      };

      class FunctionMultiplier
      {
      private:
        boost::function<void()> f;
        unsigned int            i;

      public:
        FunctionMultiplier(boost::function<void()> const& f, unsigned int i);

        FunctionMultiplier& operator*(unsigned int i);
        void                operator()();
      };
    } // namespace ThreadPool
  }   // namespace Detail
} // namespace Scroom

Scroom::Detail::ThreadPool::FunctionAdditor     operator+(boost::function<void()> const& f1, boost::function<void()> const& f2);
Scroom::Detail::ThreadPool::FunctionAdditor&    operator+(boost::function<void()> const&               f1,
                                                       Scroom::Detail::ThreadPool::FunctionAdditor& f2);
Scroom::Detail::ThreadPool::FunctionMultiplier& operator*(unsigned int i, Scroom::Detail::ThreadPool::FunctionMultiplier& m);
Scroom::Detail::ThreadPool::FunctionMultiplier  operator*(unsigned int i, boost::function<void()> const& f);
Scroom::Detail::ThreadPool::FunctionMultiplier  operator*(boost::function<void()> const& f, unsigned int i);
