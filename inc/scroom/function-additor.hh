/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2013 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef FUNCTION_ADDITOR_HH
#define FUNCTION_ADDITOR_HH

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
        std::list<boost::function<void ()> > functions;
 
      public:
        FunctionAdditor();
        ~FunctionAdditor();

        void addBefore(boost::function<void ()> const& fn);
        void addAfter(boost::function<void ()> const& fn);

        FunctionAdditor& operator+(boost::function<void ()> const& fn);
        FunctionAdditor& operator+=(boost::function<void ()> const& fn);
        void operator()();
      };

      class FunctionMultiplier
      {
      private:
        boost::function<void ()> f;
        unsigned int i;

      public:
        FunctionMultiplier(boost::function<void ()> const& f, unsigned int i);
        ~FunctionMultiplier();

        FunctionMultiplier& operator*(unsigned int i);
        void operator()();
  
      };
    }
  }
}
      
Scroom::Detail::ThreadPool::FunctionAdditor operator+(boost::function<void ()> const& f1, boost::function<void ()> const& f2);
Scroom::Detail::ThreadPool::FunctionAdditor& operator+(boost::function<void ()> const& f1, Scroom::Detail::ThreadPool::FunctionAdditor& f2);
Scroom::Detail::ThreadPool::FunctionMultiplier& operator*(unsigned int i, Scroom::Detail::ThreadPool::FunctionMultiplier m);
Scroom::Detail::ThreadPool::FunctionMultiplier operator*(unsigned int i, boost::function<void ()> const& f);
Scroom::Detail::ThreadPool::FunctionMultiplier operator*(boost::function<void ()> const& f, unsigned int i);

#endif
