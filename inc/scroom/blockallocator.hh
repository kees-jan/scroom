/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstdint>
#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/interface.hh>

namespace Scroom
{
  namespace MemoryBlocks
  {
    namespace RawPageData
    {
      using Ptr     = boost::shared_ptr<uint8_t>;
      using WeakPtr = boost::weak_ptr<uint8_t>;
    } // namespace RawPageData

    class BlockInterface;

    class Page
    {
    private:
      boost::shared_ptr<BlockInterface> bi;
      size_t                            id;

    public:
      Page(boost::shared_ptr<BlockInterface> bi, size_t id);

      RawPageData::Ptr get();
    };

    using PageList = std::list<Page>;

    class BlockInterface : private Interface
    {
    public:
      using Ptr     = boost::shared_ptr<BlockInterface>;
      using WeakPtr = boost::weak_ptr<BlockInterface>;

    protected:
      virtual RawPageData::Ptr get(size_t id) = 0;

    public:
      virtual PageList getPages() = 0;

      friend class Page;
    };

    class BlockFactoryInterface : private Interface
    {
    public:
      using Ptr = boost::shared_ptr<BlockFactoryInterface>;

      virtual BlockInterface::Ptr create(size_t count, size_t size) = 0;
    };

    BlockFactoryInterface::Ptr getBlockFactoryInterface();

    ////////////////////////////////////////////////////////////////////////
    // implementation

    inline Page::Page(BlockInterface::Ptr bi_, size_t id_)
      : bi(bi_)
      , id(id_)
    {}

    inline RawPageData::Ptr Page::get() { return bi->get(id); }
  } // namespace MemoryBlocks
} // namespace Scroom
