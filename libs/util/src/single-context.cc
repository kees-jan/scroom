/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "single-context.hh"

#include <utility>

namespace Scroom::Utils
{

  void SingleContext::set(std::string name, std::any value)
  {
    auto [it, added] = content.insert({std::move(name), std::move(value)});
    if(!added)
    {
      throw name_exists();
    }
  }

  const std::any& SingleContext::get(std::string name) const
  {
    auto item = content.find(name);

    if(item == content.end())
    {
      throw name_not_found();
    }

    return item->second;
  }

  std::any SingleContext::try_get(std::string name) const
  {
    auto item = content.find(name);

    if(item == content.end())
    {
      return {};
    }

    return item->second;
  }
} // namespace Scroom::Utils