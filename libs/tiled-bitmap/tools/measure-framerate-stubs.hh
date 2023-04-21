/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include <scroom/progressinterface.hh>
#include <scroom/tile.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/viewinterface.hh>

class PresentationInterface;

class ProgressInterfaceStub : public ProgressInterface
{
public:
  using Ptr = std::shared_ptr<ProgressInterfaceStub>;

private:
  bool finished{false};

private:
  ProgressInterfaceStub() = default;

public:
  static Ptr create();

  [[nodiscard]] bool isFinished() const;

  // ProgressInterface ///////////////////////////////////////////////////
  void setIdle() override {}
  void setWaiting(double /*progress*/) override {}
  void setWorking(double /*progress*/) override {}
  void setFinished() override;
};

class ViewInterfaceStub : public ViewInterface
{
public:
  using Ptr = std::shared_ptr<ViewInterfaceStub>;

private:
  ProgressInterface::Ptr pi;

private:
  explicit ViewInterfaceStub(ProgressInterface::Ptr pi);

public:
  static Ptr                             create(ProgressInterface::Ptr pi);
  void                                   invalidate() override {}
  ProgressInterface::Ptr                 getProgressInterface() override;
  void                                   addSideWidget(std::string /*title*/, GtkWidget* /*w*/) override {}
  void                                   removeSideWidget(GtkWidget* /*w*/) override {}
  void                                   addToToolbar(GtkToolItem* /*ti*/) override {}
  void                                   removeFromToolbar(GtkToolItem* /*ti*/) override {}
  void                                   registerSelectionListener(SelectionListener::Ptr /*unused*/) override{};
  void                                   registerPostRenderer(PostRenderer::Ptr /*unused*/) override{};
  void                                   setStatusMessage(const std::string& /*unused*/) override{};
  std::shared_ptr<PresentationInterface> getCurrentPresentation() override { return {}; };
  void                                   addToolButton(GtkToggleButton* /*unused*/, ToolStateListener::Ptr /*unused*/) override{};
};

class Source1Bpp : public SourcePresentation
{
public:
  void        fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) override;
  void        done() override{};
  std::string getName() override { return "Source1Bpp"; }
};

class Source2Bpp : public SourcePresentation
{
public:
  void        fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) override;
  void        done() override{};
  std::string getName() override { return "Source2Bpp"; }
};

class Source4Bpp : public SourcePresentation
{
public:
  void        fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) override;
  void        done() override{};
  std::string getName() override { return "Source4Bpp"; }
};

class Source8Bpp : public SourcePresentation
{
public:
  void        fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) override;
  void        done() override{};
  std::string getName() override { return "Source8Bpp"; }
};
