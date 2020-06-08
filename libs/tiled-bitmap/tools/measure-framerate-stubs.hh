/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/viewinterface.hh>
#include <scroom/tiledbitmapinterface.hh>

class ProgressInterfaceStub : public ProgressInterface
{
public:
  typedef boost::shared_ptr<ProgressInterfaceStub> Ptr;

private:
  bool finished;

private:
  ProgressInterfaceStub();

public:
  static Ptr create();

  bool isFinished();

  // ProgressInterface ///////////////////////////////////////////////////
  virtual void setIdle() {}
  virtual void setWaiting(double) {}
  virtual void setWorking(double) {}
  virtual void setFinished();
};

class ViewInterfaceStub : public ViewInterface
{
public:
  typedef boost::shared_ptr<ViewInterfaceStub> Ptr;
private:
  ProgressInterface::Ptr pi;
private:
  ViewInterfaceStub(ProgressInterface::Ptr pi);
public:
  static Ptr create(ProgressInterface::Ptr pi);
  virtual void invalidate()                           {}
  virtual ProgressInterface::Ptr getProgressInterface();
  virtual void addSideWidget(std::string, GtkWidget*) {}
  virtual void removeSideWidget(GtkWidget*)           {}
  virtual void addToToolbar(GtkToolItem*)             {}
  virtual void removeFromToolbar(GtkToolItem*)        {}
  virtual void registerSelectionListener(SelectionListener::Ptr) {};
  virtual void registerPostRenderer(PostRenderer::Ptr) {};
  virtual void setStatusMessage(const std::string&) {};
  virtual boost::shared_ptr<PresentationInterface> getCurrentPresentation() { return boost::shared_ptr<PresentationInterface>(); };
  virtual void addToolButton(GtkToggleButton*, ToolStateListener::Ptr) {};
};

class Source1Bpp : public SourcePresentation
{
public:
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);
  virtual void done() {};
};

class Source2Bpp : public SourcePresentation
{
public:
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);
  virtual void done() {};
};

class Source4Bpp : public SourcePresentation
{
public:
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);
  virtual void done() {};
};

class Source8Bpp : public SourcePresentation
{
public:
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);
  virtual void done() {};
};

