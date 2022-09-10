/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>
#include <string>

#include <scroom/interface.hh>

const std::string METADATA_PROPERTY_NAME = "Metadata";

class ShowMetadataInterface : private Interface
{
public:
  using Ptr = std::shared_ptr<ShowMetadataInterface>;

  /**
   * Request to show a window containing the presentation metadata
   *
   * @param parent the parent window, if found. Nullptr otherwise
   */
  virtual void showMetadata(GtkWindow* parent) = 0;
};
