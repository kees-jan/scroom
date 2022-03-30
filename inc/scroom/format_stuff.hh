/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <fmt/format.h>

#include <scroom/point.hh>
#include <scroom/rectangle.hh>

template <typename T>
struct fmt::formatter<Scroom::Utils::Rectangle<T>> : formatter<T>
{
  template <typename FormatContext>
  auto format(const Scroom::Utils::Rectangle<T>& r, FormatContext& ctx) -> decltype(ctx.out())
  {
    format_to(ctx.out(), "{{l: ");
    formatter<T>::format(r.getLeft(), ctx);
    format_to(ctx.out(), ", t: ");
    formatter<T>::format(r.getTop(), ctx);
    format_to(ctx.out(), ", r: ");
    formatter<T>::format(r.getRight(), ctx);
    format_to(ctx.out(), ", b: ");
    formatter<T>::format(r.getBottom(), ctx);
    format_to(ctx.out(), "}}");

    return ctx.out();
  }
};

template <typename T>
struct fmt::formatter<Scroom::Utils::Point<T>> : formatter<T>
{
  template <typename FormatContext>
  auto format(const Scroom::Utils::Point<T>& p, FormatContext& ctx) -> decltype(ctx.out())
  {
    format_to(ctx.out(), "{{");
    formatter<T>::format(p.x, ctx);
    format_to(ctx.out(), ", ");
    formatter<T>::format(p.y, ctx);
    format_to(ctx.out(), "}}");

    return ctx.out();
  }
};