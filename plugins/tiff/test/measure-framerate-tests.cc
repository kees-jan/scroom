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

#include "measure-framerate-tests.hh"

#include <time.h>

#include "measure-framerate-callbacks.hh"


////////////////////////////////////////////////////////////////////////

class Sleep
{
private:
  unsigned int secs;
  bool started;
  struct timespec t;
public:
  Sleep(unsigned int secs);

  bool operator()();
};

////////////////////////////////////////////////////////////////////////

static bool quit()
{
  gtk_main_quit();

  return false;
}

Sleep::Sleep(unsigned int secs)
  : secs(secs), started(false)
{
}

bool Sleep::operator()()
{
  if(!started && 0==clock_gettime(CLOCK_REALTIME, &t))
  {
    started = true;
    return true;
  }

  struct timespec now;
  if(0==clock_gettime(CLOCK_REALTIME, &now))
  {
    if(now.tv_sec > t.tv_sec + secs)
      return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////

void init_tests()
{
  functions.push_back(Sleep(60));
  functions.push_back(quit);
}
