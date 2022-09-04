/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstdio>
#include <list>
#include <string>
#include <utility>

#include <spdlog/spdlog.h>

#include <glib.h>

#include <scroom/utilities.hh>

using namespace Scroom::Utils;

///////////////////////////////////////////////////////////////////////

void Scroom::Utils::dumpCounts() { Counter::instance()->dump(); }

gboolean timedDumpCounts(gpointer data)
{
  static_cast<Counter*>(data)->dump();
  return true;
}

///////////////////////////////////////////////////////////////////////

Count::Count(std::string name_)
  : name(std::move(name_))

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
  static auto* me = new Counter();
  return me;
}

Counter::Counter() { g_timeout_add_seconds(10, timedDumpCounts, this); }

void Counter::registerCount(const Count::Ptr& count)
{
  boost::unique_lock<boost::mutex> const lock(mut);
  counts.push_back(count);
}

void Counter::unregisterCount(const Count::Ptr& count)
{
  boost::unique_lock<boost::mutex> const lock(mut);
  counts.remove(count);
}

void Counter::dump()
{
  boost::unique_lock<boost::mutex> const lock(mut);
  std::stringstream                      out;

  out << counts.size();
  for(Count::Ptr const& count: counts)
  {
    out << ", " << count->name << ", " << count->count;
  }
  spdlog::trace("{}", out.str());
}

std::list<Count::Ptr> Counter::getCounts()
{
  boost::unique_lock<boost::mutex> const lock(mut);
  return counts;
}
