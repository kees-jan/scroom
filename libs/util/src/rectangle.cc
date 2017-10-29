/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/rectangle.hh>

std::ostream& operator<<(std::ostream& os, const Rectangle& r)
{
  os << '<' << r.getLeftPos()
     << ',' << r.getTopPos()
     << ',' << r.getWidth()
     << ',' << r.getHeight()
     << '>';

  return os;
}
