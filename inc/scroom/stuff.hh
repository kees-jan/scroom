/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace Scroom
{
  namespace Utils
  {
    /** Stuff is a pointer to some private data. */
    typedef boost::shared_ptr<void> Stuff;
    typedef boost::weak_ptr<void>   StuffWeak;
    typedef std::list<Stuff>        StuffList;
  } // namespace Utils
} // namespace Scroom
