/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>

#include <fmt/format.h>

#include <scroom/point.hh>
#include <scroom/rectangle.hh>

template <typename T>
struct fmt::formatter<Scroom::Utils::Rectangle<T>> : formatter<T>
{
  template <typename FormatContext>
  auto format(const Scroom::Utils::Rectangle<T>& r, FormatContext& ctx) const -> decltype(ctx.out())
  {
    format_to(ctx.out(), "<");
    formatter<T>::format(r.getLeft(), ctx);
    format_to(ctx.out(), ",");
    formatter<T>::format(r.getTop(), ctx);
    format_to(ctx.out(), ",");
    formatter<T>::format(r.getWidth(), ctx);
    format_to(ctx.out(), ",");
    formatter<T>::format(r.getHeight(), ctx);
    format_to(ctx.out(), ">");

    return ctx.out();
  }
};

template <typename T>
struct fmt::formatter<Scroom::Utils::Point<T>> : formatter<T>
{
  template <typename FormatContext>
  auto format(const Scroom::Utils::Point<T>& p, FormatContext& ctx) const -> decltype(ctx.out())
  {
    format_to(ctx.out(), "(");
    formatter<T>::format(p.x, ctx);
    format_to(ctx.out(), ",");
    formatter<T>::format(p.y, ctx);
    format_to(ctx.out(), ")");

    return ctx.out();
  }
};

template <typename T>
struct fmt::formatter<std::shared_ptr<T>> : formatter<const void*>
{
  template <typename FormatContext>
  auto format(const std::shared_ptr<T>& p, FormatContext& ctx) const -> decltype(ctx.out())
  {
    return formatter<const void*>::format(fmt::ptr(p.get()), ctx);
  }
};

template <typename T>
struct fmt::formatter<std::weak_ptr<T>> : formatter<const void*>
{
  template <typename FormatContext>
  auto format(const std::weak_ptr<T>& p, FormatContext& ctx) const -> decltype(ctx.out())
  {
    auto locked = p.lock();
    if(locked)
    {
      return formatter<const void*>::format(fmt::ptr(locked.get()), ctx);
    }

    return format_to(ctx.out(), "[expired]");
  }
};