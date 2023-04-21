/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <utility>

#include <scroom/interface.hh>

namespace Scroom::MemoryBlocks
{
  namespace RawPageData
  {
    using Ptr     = std::shared_ptr<uint8_t>;
    using WeakPtr = std::weak_ptr<uint8_t>;
  } // namespace RawPageData

  class BlockInterface;

  class Page
  {
  private:
    std::shared_ptr<BlockInterface> bi;
    size_t                          id;

  public:
    Page(std::shared_ptr<BlockInterface> bi, size_t id);

    RawPageData::Ptr get();
  };

  using PageList = std::list<Page>;

  class BlockInterface : private Interface
  {
  public:
    using Ptr     = std::shared_ptr<BlockInterface>;
    using WeakPtr = std::weak_ptr<BlockInterface>;

  protected:
    virtual RawPageData::Ptr get(size_t id) = 0;

  public:
    virtual PageList getPages() = 0;

    friend class Page;
  };

  class BlockFactoryInterface : private Interface
  {
  public:
    using Ptr = std::shared_ptr<BlockFactoryInterface>;

    virtual BlockInterface::Ptr create(size_t count, size_t size) = 0;
  };

  BlockFactoryInterface::Ptr getBlockFactoryInterface();

  ////////////////////////////////////////////////////////////////////////
  // implementation

  inline Page::Page(BlockInterface::Ptr bi_, size_t id_)
    : bi(std::move(bi_))
    , id(id_)
  {
  }

  inline RawPageData::Ptr Page::get() { return bi->get(id); }
} // namespace Scroom::MemoryBlocks
