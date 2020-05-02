/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <stdint.h>

namespace Scroom
{
  namespace MemoryBlocks
  {
    namespace RawPageData
    {
      typedef boost::shared_ptr<uint8_t> Ptr;
      typedef boost::weak_ptr<uint8_t> WeakPtr;
    }

    class BlockInterface;

    class Page
    {
    private:
      boost::shared_ptr<BlockInterface> bi;
      size_t id;

    public:
      Page(boost::shared_ptr<BlockInterface> bi, size_t id);

      RawPageData::Ptr get();
    };

    typedef std::list<Page> PageList;

    class BlockInterface
    {
    public:
      typedef boost::shared_ptr<BlockInterface> Ptr;
      typedef boost::weak_ptr<BlockInterface> WeakPtr;

    protected:
      virtual RawPageData::Ptr get(size_t id)=0;

    public:
      virtual ~BlockInterface() {}

      virtual PageList getPages()=0;

      friend class Page;
    };

    class BlockFactoryInterface
    {
    public:
      typedef boost::shared_ptr<BlockFactoryInterface> Ptr;

      virtual ~BlockFactoryInterface() {}
      virtual BlockInterface::Ptr create(size_t count, size_t size)=0;
    };

    BlockFactoryInterface::Ptr getBlockFactoryInterface();

    ////////////////////////////////////////////////////////////////////////
    // implementation

    inline Page::Page(BlockInterface::Ptr bi_, size_t id_)
      : bi(bi_), id(id_)
    {}

    inline RawPageData::Ptr Page::get()
    { return bi->get(id); }
  }
}

