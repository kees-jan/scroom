/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/context.hh>

#include "parent-context.hh"
#include "single-context.hh"

namespace Scroom::Utils
{
  Context::Ptr          Context::Create() { return std::make_shared<SingleContext>(); }
  RecursiveContext::Ptr RecursiveContext::Create() { return std::make_shared<ParentContext>(Context::Create()); }
} // namespace Scroom::Utils
