/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <string_view>

#include <sys/cdefs.h>

namespace Scroom::Utils::Detail
{
  /** Gets called when an assertion failed */
  [[noreturn]] void assertionFailed(const std::string_view type,
                                    const std::string_view expr,
                                    const std::string_view function,
                                    const std::string_view filename,
                                    unsigned int           line) __attribute__((noreturn));
} // namespace Scroom::Utils::Detail

#ifdef _WIN32
#  define __STRING(x) #  x
#endif

#define require(expr)                               \
  ((expr) ? ((void)0)                               \
          : Scroom::Utils::Detail::assertionFailed( \
            "precondition", __STRING(expr), static_cast<const char*>(__PRETTY_FUNCTION__), __FILE__, __LINE__))
#define ensure(expr)                                \
  ((expr) ? ((void)0)                               \
          : Scroom::Utils::Detail::assertionFailed( \
            "postcondition", __STRING(expr), static_cast<const char*>(__PRETTY_FUNCTION__), __FILE__, __LINE__))
#define verify(expr)                                \
  ((expr) ? ((void)0)                               \
          : Scroom::Utils::Detail::assertionFailed( \
            "assertion", __STRING(expr), static_cast<const char*>(__PRETTY_FUNCTION__), __FILE__, __LINE__))
#define defect()                          \
  Scroom::Utils::Detail::assertionFailed( \
    "control flow assertion", "", static_cast<const char*>(__PRETTY_FUNCTION__), __FILE__, __LINE__)
#define defect_message(m)                 \
  Scroom::Utils::Detail::assertionFailed( \
    "control flow assertion", (m), static_cast<const char*>(__PRETTY_FUNCTION__), __FILE__, __LINE__)
