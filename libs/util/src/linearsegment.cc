/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/linearsegment.hh>

std::ostream& operator<<(std::ostream& os, const Segment& s)
{
  return os << '(' << s.getStart()
            << ',' << s.getSize()
            << ')';
}

