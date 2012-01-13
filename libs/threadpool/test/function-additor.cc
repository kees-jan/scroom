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

#include "function-additor.hh"

#include <boost/foreach.hpp>

FunctionAdditor::FunctionAdditor()
{}

FunctionAdditor::~FunctionAdditor()
{}

FunctionAdditor& FunctionAdditor::operator+(boost::function<void ()> const& fn)
{
  functions.push_back(fn);

  return *this;
}

void FunctionAdditor::operator()()
{
  BOOST_FOREACH(boost::function<void ()>& f, functions)
  {
    f();
  }
}

////////////////////////////////////////////////////////////////////////

FunctionMultiplier::FunctionMultiplier(boost::function<void ()> const& f, unsigned int i)
  : f(f), i(i)
{}

FunctionMultiplier::~FunctionMultiplier()
{}

FunctionMultiplier& FunctionMultiplier::operator*(unsigned int i)
{
  this->i *= i;
  return *this;
}

void FunctionMultiplier::operator()()
{
  for(unsigned int c=0; c<i; c++)
    f();
}

////////////////////////////////////////////////////////////////////////

FunctionAdditor operator+(boost::function<void ()> const& f1, boost::function<void ()> const& f2)
{
  return FunctionAdditor() + f1 + f2;
}

FunctionMultiplier& operator*(unsigned int i, FunctionMultiplier m)
{
  return m*i;
}

FunctionMultiplier operator*(unsigned int i, boost::function<void ()> const& f)
{
  return FunctionMultiplier(f, i);
}

FunctionMultiplier operator*(boost::function<void ()> const& f, unsigned int i)
{
  return FunctionMultiplier(f, i);
}
