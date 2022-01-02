/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <string>

#include <spdlog/spdlog.h>
#include <time.h>

namespace Scroom
{
  namespace Utils
  {
    class Timer
    {
    private:
      struct timespec t;
      std::string     label;
      bool            valid;

    public:
      Timer(std::string label)
        : label(label)
      {
        valid = (0 == clock_gettime(CLOCK_MONOTONIC, &t));
      }

      ~Timer()
      {
        struct timespec t2;
        bool            v2 = (0 == clock_gettime(CLOCK_MONOTONIC, &t2));
        if(valid && v2)
        {
          double elapsed = (t2.tv_nsec - t.tv_nsec) * 1e-9;
          elapsed += t2.tv_sec - t.tv_sec;
          spdlog::trace("{}: {:.9f}", label, elapsed);
        }
        else
        {
          spdlog::error("{}: Clock invalid", label);
        }
      }
    };
  } // namespace Utils
} // namespace Scroom
