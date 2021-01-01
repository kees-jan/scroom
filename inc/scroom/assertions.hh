/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <sys/cdefs.h>

namespace Scroom
{
  namespace Utils
  {

    namespace Detail
    {

      /** Gets called when an assertion failed */
      [[noreturn]] void
        assertionFailed(const char* type, const char* expr, const char* function, const char* filename, unsigned int line)
          __attribute__((noreturn));
    } // namespace Detail

  } // namespace Utils
} // namespace Scroom

#ifdef _WIN32
#  define __STRING(x) #  x
#endif

// NOLINTNEXTLINE
#define require(expr) \
  ((expr) ? ((void)0) \
          : Scroom::Utils::Detail::assertionFailed("precondition", __STRING(expr), __PRETTY_FUNCTION__, __FILE__, __LINE__))
// NOLINTNEXTLINE
#define ensure(expr)  \
  ((expr) ? ((void)0) \
          : Scroom::Utils::Detail::assertionFailed("postcondition", __STRING(expr), __PRETTY_FUNCTION__, __FILE__, __LINE__))
// NOLINTNEXTLINE
#define verify(expr)  \
  ((expr) ? ((void)0) \
          : Scroom::Utils::Detail::assertionFailed("assertion", __STRING(expr), __PRETTY_FUNCTION__, __FILE__, __LINE__))
// NOLINTNEXTLINE
#define defect() Scroom::Utils::Detail::assertionFailed("control flow assertion", "", __PRETTY_FUNCTION__, __FILE__, __LINE__)
