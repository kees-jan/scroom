/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/assertions.hh>
#include <scroom/tile-storage.hh>

#include "tileviewstate.hh"

TileViewState::Ptr TileStorage::getViewState(const ViewInterface::WeakPtr& vi)
{
  TileViewState::Ptr result = m_viewStates[vi].lock();

  if(!result)
  {
    result = TileViewState::create(shared_from_this<TileStorage>());
    m_viewStates[vi] = result;
  }

  return result;
}


void TileStorage::open(ViewInterface::WeakPtr /*view*/) {}
void TileStorage::close(ViewInterface::WeakPtr view) { m_viewStates.erase(view); }

void TileStorage::observerAdded(const TileInitialisationObserver::Ptr& observer, const Scroom::Bookkeeping::Token& /*token*/)
{
  observer->tileCreated(shared_from_this<TileStorage>());
}

void TileStorage::observerAdded(const TileLoadingObserver::Ptr& observer, const Scroom::Bookkeeping::Token& token)
{
  if(const auto pixels = getConstTileAsync())
  {
    observer->tileLoaded(pixels);
    token.add(pixels);
  }
}

void TileStorage::reportFinished()
{
  const auto pixels = getConstTileSync();
  require(pixels);
  const auto me = shared_from_this<TileStorage>();
  for(const auto& observer: Scroom::Utils::Observable<TileInitialisationObserver>::getObservers())
  {
    observer->tileFinished(me);
  }
  for(const auto& observer: Scroom::Utils::Observable<TileLoadingObserver>::getObservers())
  {
    observer->tileLoaded(pixels);
  }
}

void TileInitialisationObserver::tileFinished(const TileStorage::Ptr& /*tile*/) {}
void TileInitialisationObserver::tileCreated(const TileStorage::Ptr& /*tile*/) {}
