/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <fstream>
#include <map>
#include <string>

#include <scroom/logger.hh>
#include <scroom/opentiledbitmapinterface.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/transformpresentation.hh>

namespace Scroom::Pnm
{
  using namespace Scroom::TiledBitmap;

  enum class SourceType
  {
    Binary,
    Ascii,
    Ascii1bpp
  };

  boost::optional<std::tuple<Scroom::TiledBitmap::BitmapMetaData, std::ifstream, SourceType>>
    open(const Scroom::Logger& logger, const std::string& fileName);

  class Source : public SourcePresentation
  {
  public:
    using Ptr = std::shared_ptr<Source>;

  protected:
    std::string m_fileName;
    std::ifstream m_preOpenedPnm;
    std::ifstream m_pnm;
    BitmapMetaData m_bmd;
    Scroom::Logger m_logger;

  public:
    bool resetPresentation();

    // SourcePresentation
    void done() override;
    std::string getName() override { return m_fileName; }

  protected:
    Source(std::string fileName, std::ifstream pnm, BitmapMetaData bmd);
  };

  class BinarySource : public Source
  {
  public:
    using Ptr = std::shared_ptr<BinarySource>;

    static Ptr create(std::string fileName, std::ifstream pnm, BitmapMetaData bmd);

    // SourcePresentation
    void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) override;

  private:
    using Source::Source;
  };

  class AsciiSource : public Source
  {
  public:
    using Ptr = std::shared_ptr<AsciiSource>;

    static Ptr create(std::string fileName, std::ifstream pnm, BitmapMetaData bmd);

    // SourcePresentation
    void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) override;

  private:
    using Source::Source;
  };

  class AsciiSource1bpp : public Source
  {
  public:
    using Ptr = std::shared_ptr<AsciiSource1bpp>;

    static Ptr create(std::string fileName, std::ifstream pnm, BitmapMetaData bmd);

    // SourcePresentation
    void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) override;

  private:
    using Source::Source;
  };
} // namespace Scroom::Pnm
