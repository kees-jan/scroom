/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
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
    using Stuff     = boost::shared_ptr<void>;
    using StuffWeak = boost::weak_ptr<void>;
    using StuffList = std::list<Stuff>;
  } // namespace Utils
} // namespace Scroom
