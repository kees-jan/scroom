/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
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
#include <scroom/utilities.hh>

#include <stdio.h>

#include <glib.h>

#include <boost/foreach.hpp>

using namespace Scroom::Utils;

///////////////////////////////////////////////////////////////////////

void Scroom::Utils::dumpCounts()
{
  Counter::instance()->dump();
}

gboolean timedDumpCounts(gpointer data)
{
  Counter* c = (Counter*) data;
  c->dump();
  return true;
}

///////////////////////////////////////////////////////////////////////

Count::Count(const std::string& name)
  : name(name), mut(), count(0)
{
}

Count::Ptr Count::create(const std::string& name)
{
  Ptr result = Ptr(new Count(name));
  Counter::instance()->registerCount(result);
  return result;
}

///////////////////////////////////////////////////////////////////////

Counter* Counter::instance()
{
  static Counter* me = new Counter();
  return me;
}

Counter::Counter()
{
  g_timeout_add_seconds(10, timedDumpCounts, this);
}

void Counter::registerCount(Count::Ptr count)
{
  boost::unique_lock<boost::mutex> lock(mut);
  counts.push_back(count);
}

void Counter::unregisterCount(Count::Ptr count)
{
  boost::unique_lock<boost::mutex> lock(mut);
  counts.remove(count);
}

void Counter::dump()
{
  boost::unique_lock<boost::mutex> lock(mut);
  printf("%ld", (long)counts.size());
  BOOST_FOREACH(Count::Ptr& c, counts)
  {
    printf(", %s, %ld", c->name.c_str(), c->count);
  }
  printf("\n");
}

std::list<Count::Ptr> Counter::getCounts()
{
  boost::unique_lock<boost::mutex> lock(mut);
  return counts;
}
