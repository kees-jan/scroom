/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <string>
#include <vector>

namespace Scroom::Metadata
{
  using Metadata = std::vector<std::pair<std::string, std::string>>;

  void showMetaData(GtkWindow* parent, std::string title, Metadata data);
} // namespace Scroom::Metadata