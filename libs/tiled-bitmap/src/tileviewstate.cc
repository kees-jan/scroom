/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "tileviewstate.hh"

#include <scroom/async-deleter.hh>

#include "tileinternal.hh"

TileViewState::~TileViewState()
{
  r.reset();
}

TileViewState::Ptr TileViewState::create(boost::shared_ptr<TileInternal> parent)
{
  TileViewState::Ptr result(new TileViewState(parent));

  result->r = parent->registerObserver(result);

  return result;
}

TileViewState::TileViewState(boost::shared_ptr<TileInternal> parent)
  : parent(parent), state(INIT), desiredState(BASE_COMPUTED)
{
}

void TileViewState::tileLoaded(Tile::Ptr tile)
{
  boost::mutex::scoped_lock l(mut);
  this->tile = tile;
}

Scroom::Utils::Registration TileViewState::getCacheResult()
{
  return Scroom::Utils::Registration();
}
