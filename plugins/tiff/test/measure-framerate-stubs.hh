/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

#ifndef MEASURE_FRAMERATE_STUBS_HH
#define MEASURE_FRAMERATE_STUBS_HH

#include <scroom/viewinterface.hh>
#include <scroom/tiledbitmapinterface.hh>

class ProgressInterfaceStub : public ProgressInterface
{
private:
  bool finished;
public:
  ProgressInterfaceStub();
  virtual void setState(State state);      
  virtual void setProgress(double)   {}
  virtual void setProgress(int, int) {}

  bool isFinished();
};

class ViewInterfaceStub : public ViewInterface
{
private:
  ProgressInterface* pi;
public:
  ViewInterfaceStub(ProgressInterface* pi);
  virtual void invalidate()                           {}
  virtual ProgressInterface* getProgressInterface();
  virtual void addSideWidget(std::string, GtkWidget*) {}
  virtual void removeSideWidget(GtkWidget*)           {}
  virtual void addToToolbar(GtkToolItem*)             {}
  virtual void removeFromToolbar(GtkToolItem*)        {}
};

class Source1Bpp : public SourcePresentation
{
public:
  virtual ~Source1Bpp()
  {}

  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);
  virtual void done() {};
};

class Source2Bpp : public SourcePresentation
{
public:
  virtual ~Source2Bpp()
  {}

  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);
  virtual void done() {};
};

class Source4Bpp : public SourcePresentation
{
public:
  virtual ~Source4Bpp()
  {}

  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);
  virtual void done() {};
};

class Source8Bpp : public SourcePresentation
{
public:
  virtual ~Source8Bpp()
  {}

  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);
  virtual void done() {};
};

#endif
