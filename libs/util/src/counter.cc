/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/utilities.hh>

#include <stdio.h>

#include <glib.h>

using namespace Scroom::Utils;

///////////////////////////////////////////////////////////////////////

void Scroom::Utils::dumpCounts()
{
  Counter::instance()->dump();
}

gboolean timedDumpCounts(gpointer data)
{
  static_cast<Counter*>(data)->dump();
  return true;
}

///////////////////////////////////////////////////////////////////////

Count::Count(const std::string& name_)
  : name(name_), mut(), count(0)
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
  printf("%zu", counts.size());
  for(Count::Ptr& c: counts)
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
