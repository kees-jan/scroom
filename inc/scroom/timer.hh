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

#ifndef TIMER_HH
#define TIMER_HH

#include <time.h>

#include <string>

namespace Scroom
{
  namespace Utils
  {
    class Timer
    {
    private:
      struct timespec t;
      std::string label;
      bool valid;
      
    public:
      Timer(std::string label)
        : label(label)
      {
        valid = (0==clock_gettime(CLOCK_MONOTONIC, &t));
      }

      ~Timer()
      {
        struct timespec t2;
        bool v2 = (0==clock_gettime(CLOCK_MONOTONIC, &t2));
        if(valid && v2)
        {
          double elapsed = (t2.tv_nsec - t.tv_nsec)*1e-9;
          elapsed += t2.tv_sec - t.tv_sec;
          printf("%s: %.9f\n", label.c_str(), elapsed);
        }
        else
        {
          printf("%s: Clock invalid\n", label.c_str());
        }
      }
    };
  }
}


#endif
