/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/layer-base.hh>

Scroom::Bookkeeping::Token LayerBase::registerObserver(const TileInitialisationObserver::Ptr& observer)
{
  Scroom::Bookkeeping::Token registrations;
  for(const auto index: getTileRange())
  {
    registrations.add(getTile(index)->registerObserver(observer));
  }
  return registrations;
}

void LayerBase::open(ViewInterface::WeakPtr view)
{
  for(const auto index: getTileRange())
  {
    getTile(index)->open(view);
  }
  getOutOfBoundTile()->open(view);
}

void LayerBase::close(ViewInterface::WeakPtr view)
{
  for(const auto index: getTileRange())
  {
    getTile(index)->close(view);
  }
  getOutOfBoundTile()->close(view);
}
