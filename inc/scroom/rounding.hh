/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cmath>

inline double rounded_divide_by(double value, double factor) { return std::round(value / factor); }
inline double ceiled_divide_by(double value, double factor) { return std::ceil(value / factor); }
inline double floored_divide_by(double value, double factor) { return std::floor(value / factor); }

inline int rounded_divide_by(int value, int factor)
{
  return static_cast<int>(rounded_divide_by(static_cast<double>(value), static_cast<double>(factor)));
}

inline int ceiled_divide_by(int value, int factor)
{
  return static_cast<int>(ceiled_divide_by(static_cast<double>(value), static_cast<double>(factor)));
}

inline int floored_divide_by(int value, int factor)
{
  return static_cast<int>(floored_divide_by(static_cast<double>(value), static_cast<double>(factor)));
}

template <typename T>
T round_to_multiple_of(T value, T factor)
{
  return factor * rounded_divide_by(value, factor);
}

template <typename T>
T round_up_to_multiple_of(T value, T factor)
{
  return factor * ceiled_divide_by(value, factor);
}

template <typename T>
T round_down_to_multiple_of(T value, T factor)
{
  return factor * floored_divide_by(value, factor);
}
