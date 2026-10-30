/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <gmock/gmock.h>

#include <scroom/tweak-view.hh>

namespace Scroom::Utils
{
  inline testing::Matcher<const Point<double>&> pointEquals(Point<double> expected)
  {
    return testing::AllOf(
      testing::Field("x", &Point<double>::x, testing::DoubleEq(expected.x)),
      testing::Field("y", &Point<double>::y, testing::DoubleEq(expected.y))
    );
  }

  inline testing::Matcher<const Selection&> selectionEquals(const Selection& expected)
  {
    return testing::AllOf(
      testing::Field("start", &Selection::start, pointEquals(expected.start)),
      testing::Field("end", &Selection::end, pointEquals(expected.end))
    );
  }

} // namespace Scroom::Utils
