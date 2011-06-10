/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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

class FunctionAdditor
{
private:
  std::list<boost::function<void ()> > functions;

public:
  FunctionAdditor();
  ~FunctionAdditor();

  FunctionAdditor& operator+(boost::function<void ()> const& fn);
  void operator()();
};

FunctionAdditor operator+(boost::function<void ()> const& f1, boost::function<void ()> const& f2);



#endif
