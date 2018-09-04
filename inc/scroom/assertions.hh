/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <sys/cdefs.h>

#ifndef __clang__
#define GCC_VERSION (__GNUC__ * 10000           \
                     + __GNUC_MINOR__ * 100     \
                     + __GNUC_PATCHLEVEL__)
#endif

namespace Scroom
{
  namespace Utils
  {

    namespace Detail
    {

      /** Gets called when an assertion failed */
      [[noreturn]] void assertionFailed(const char *type, const char *expr,
          const char *function, const char *filename, unsigned int line)
              __attribute__ ((noreturn));
    }

  }
}

#if defined(OS_cygwin) || defined(OS_mingw) || defined(OS_msvc6)
#define __STRING(x)     #x
#endif

#define require(expr)                                                       \
  ((expr) ? ((void) 0) : Scroom::Utils::Detail::assertionFailed             \
   ("precondition", __STRING(expr), __PRETTY_FUNCTION__, __FILE__, __LINE__))
#define ensure(expr)                                                        \
  ((expr) ? ((void) 0) : Scroom::Utils::Detail::assertionFailed             \
   ("postcondition", __STRING(expr), __PRETTY_FUNCTION__,  __FILE__, __LINE__))
#define verify(expr)                                                        \
  ((expr) ? ((void) 0) : Scroom::Utils::Detail::assertionFailed             \
   ("assertion", __STRING(expr), __PRETTY_FUNCTION__,__FILE__, __LINE__))
#define defect()                                                            \
  Scroom::Utils::Detail::assertionFailed("control flow assertion",          \
                                     "", __PRETTY_FUNCTION__,__FILE__, __LINE__)
