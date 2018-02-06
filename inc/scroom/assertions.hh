/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <sys/cdefs.h>

#define GCC_VERSION (__GNUC__ * 10000           \
                     + __GNUC_MINOR__ * 100     \
                     + __GNUC_PATCHLEVEL__)

#if GCC_VERSION < 40800  // 4.8.0
#define NORETURN __attribute__((noreturn))  // precise, gcc 4.6.3
#else
#define NORETURN [[noreturn]] // others // gcc 4.8.4
#endif


namespace Scroom
{
  namespace Utils
  {

    namespace Detail
    {

      /** Gets called when an assertion failed */
      NORETURN void assertionFailed(const char *type, const char *expr,
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
