/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "measure-framerate-stubs.hh"

#include <utility>

#include <scroom/progressinterfacehelpers.hh>

ProgressInterfaceStub::Ptr ProgressInterfaceStub::create() { return Ptr(new ProgressInterfaceStub()); }

void ProgressInterfaceStub::setFinished() { finished = true; }

bool ProgressInterfaceStub::isFinished() const { return finished; }

ViewInterfaceStub::ViewInterfaceStub(ProgressInterface::Ptr pi_)
  : pi(std::move(pi_))
{
}

ViewInterfaceStub::Ptr ViewInterfaceStub::create(ProgressInterface::Ptr pi) { return Ptr(new ViewInterfaceStub(std::move(pi))); }

ProgressInterface::Ptr ViewInterfaceStub::getProgressInterface() { return pi; }

void Source1Bpp::fillTiles(int /*startLine*/, int lineCount, int tileWidth, int /*firstTile*/, std::vector<Tile::Ptr>& tiles)
{
  for(const Tile::Ptr& tile: tiles)
  {
    byte* data = tile->data.get();
    for(int y = 0; y < lineCount; y += 2)
    {
      for(int x = 0; x < tileWidth / 8; x++)
      {
        *data = 0xAA;
        data++;
      }
      for(int x = 0; x < tileWidth / 8; x++)
      {
        *data = 0x55;
        data++;
      }
    }
  }
}

void Source2Bpp::fillTiles(int /*startLine*/, int lineCount, int tileWidth, int /*firstTile*/, std::vector<Tile::Ptr>& tiles)
{
  for(const Tile::Ptr& tile: tiles)
  {
    byte* data = tile->data.get();
    for(int y = 0; y < lineCount; y++)
    {
      for(int x = 0; x < tileWidth / 4; x++)
      {
        byte const v = 4 * x + y;

        *data = ((v & 0x3) << 6) | (((v + 1) & 0x3) << 4) | (((v + 2) & 0x3) << 2) | (((v + 3) & 0x3) << 0);
        data++;
      }
    }
  }
}

void Source4Bpp::fillTiles(int /*startLine*/, int lineCount, int tileWidth, int /*firstTile*/, std::vector<Tile::Ptr>& tiles)
{
  for(const Tile::Ptr& tile: tiles)
  {
    byte* data = tile->data.get();
    for(int y = 0; y < lineCount; y++)
    {
      for(int x = 0; x < tileWidth / 2; x++)
      {
        byte const v = 2 * x + y;

        *data = ((v & 0xF) << 4) | ((v + 1) & 0xF);
        data++;
      }
    }
  }
}

void Source8Bpp::fillTiles(int /*startLine*/, int lineCount, int tileWidth, int /*firstTile*/, std::vector<Tile::Ptr>& tiles)
{
  for(const Tile::Ptr& tile: tiles)
  {
    byte* data = tile->data.get();
    for(int y = 0; y < lineCount; y++)
    {
      for(int x = 0; x < tileWidth; x++)
      {
        *data = (x + y) & 0xFF;
        data++;
      }
    }
  }
}
