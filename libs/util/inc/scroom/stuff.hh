/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <memory>


namespace Scroom::Utils
{
  /** Stuff is a pointer to some private data. */
  using Stuff     = std::shared_ptr<void>;
  using StuffWeak = std::weak_ptr<void>;
  using StuffList = std::list<Stuff>;

  inline Stuff Empty() { return {}; }
} // namespace Scroom::Utils
