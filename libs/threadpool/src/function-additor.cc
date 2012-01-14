/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

#include "scroom/function-additor.hh"

#include <boost/foreach.hpp>

Scroom::Detail::ThreadPool::FunctionAdditor::FunctionAdditor()
{}

Scroom::Detail::ThreadPool::FunctionAdditor::~FunctionAdditor()
{}

inline void Scroom::Detail::ThreadPool::FunctionAdditor::addBefore(boost::function<void ()> const& fn)
{
  functions.push_front(fn);
}

inline void Scroom::Detail::ThreadPool::FunctionAdditor::addAfter(boost::function<void ()> const& fn)
{
  functions.push_back(fn);
}

Scroom::Detail::ThreadPool::FunctionAdditor& Scroom::Detail::ThreadPool::FunctionAdditor::operator+(boost::function<void ()> const& fn)
{
  addAfter(fn);

  return *this;
}

Scroom::Detail::ThreadPool::FunctionAdditor& Scroom::Detail::ThreadPool::FunctionAdditor::operator+=(boost::function<void ()> const& fn)
{
  addAfter(fn);

  return *this;
}

void Scroom::Detail::ThreadPool::FunctionAdditor::operator()()
{
  BOOST_FOREACH(boost::function<void ()>& f, functions)
  {
    f();
  }
}

////////////////////////////////////////////////////////////////////////

Scroom::Detail::ThreadPool::FunctionMultiplier::FunctionMultiplier(boost::function<void ()> const& f, unsigned int i)
  : f(f), i(i)
{}

Scroom::Detail::ThreadPool::FunctionMultiplier::~FunctionMultiplier()
{}

Scroom::Detail::ThreadPool::FunctionMultiplier& Scroom::Detail::ThreadPool::FunctionMultiplier::operator*(unsigned int i)
{
  this->i *= i;
  return *this;
}

void Scroom::Detail::ThreadPool::FunctionMultiplier::operator()()
{
  for(unsigned int c=0; c<i; c++)
    f();
}

////////////////////////////////////////////////////////////////////////

Scroom::Detail::ThreadPool::FunctionAdditor operator+(boost::function<void ()> const& f1, boost::function<void ()> const& f2)
{
  return Scroom::Detail::ThreadPool::FunctionAdditor() + f1 + f2;
}

Scroom::Detail::ThreadPool::FunctionAdditor& operator+(boost::function<void ()> const& f1, Scroom::Detail::ThreadPool::FunctionAdditor& f2)
{
  f2.addBefore(f1);

  return f2;
}

Scroom::Detail::ThreadPool::FunctionMultiplier& operator*(unsigned int i, Scroom::Detail::ThreadPool::FunctionMultiplier m)
{
  return m*i;
}

Scroom::Detail::ThreadPool::FunctionMultiplier operator*(unsigned int i, boost::function<void ()> const& f)
{
  return Scroom::Detail::ThreadPool::FunctionMultiplier(f, i);
}

Scroom::Detail::ThreadPool::FunctionMultiplier operator*(boost::function<void ()> const& f, unsigned int i)
{
  return Scroom::Detail::ThreadPool::FunctionMultiplier(f, i);
}
