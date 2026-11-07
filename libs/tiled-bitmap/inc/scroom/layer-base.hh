/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/layer-interface.hh>

/** Forwards observer registration and Viewable calls to each tile. */
class LayerBase : public LayerInterface
{
public:
  Scroom::Bookkeeping::Token registerObserver(const TileInitialisationObserver::Ptr& observer) override;

  void open(ViewInterface::WeakPtr view) override;
  void close(ViewInterface::WeakPtr view) override;
};
