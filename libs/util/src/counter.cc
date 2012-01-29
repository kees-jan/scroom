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
#include <scroom/utilities.hh>

#include <stdio.h>

#include <glib.h>

using namespace Scroom::Utils;

///////////////////////////////////////////////////////////////////////

Counter::Ptr Scroom::Utils::getCounter()
{
  static Counter::Ptr instance = Counter::create();
  return instance;
}

void dumpCounts()
{
  getCounter()->dump();
}

gboolean timedDumpCounts(gpointer data)
{
  Counter* c = (Counter*) data;
  c->dump();
  return true;
}

///////////////////////////////////////////////////////////////////////

Counter::Ptr Counter::create()
{
  return Counter::Ptr(new Counter());
}

Counter::Counter()
{
  g_timeout_add_seconds(10, timedDumpCounts, this);
}

void Counter::registerClass(std::string name, long int& count)
{
  boost::unique_lock<boost::mutex> lock(mut);
  counts[name] = &count;
}

void Counter::unregisterClass(std::string name)
{
  boost::unique_lock<boost::mutex> lock(mut);
  counts.erase(name);
}

void Counter::dump()
{
  boost::unique_lock<boost::mutex> lock(mut);
  printf("%ld", (long)counts.size());
  for(std::map<std::string,long*>::iterator cur=counts.begin();
      cur!=counts.end();
      ++cur)
    {
      printf(", %s, %ld", cur->first.c_str(), *(cur->second));
    }
  printf("\n");
}

std::map<std::string, long*> Counter::getCounts()
{
  boost::unique_lock<boost::mutex> lock(mut);
  return counts;
}

///////////////////////////////////////////////////////////////////////

Counter::Registrar::Registrar(std::string name, long& count)
: name(name), counter(getCounter())
{
  counter->registerClass(name, count);
}

Counter::Registrar::~Registrar()
{
  counter->unregisterClass(name);
}
