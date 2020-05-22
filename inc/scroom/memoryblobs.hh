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
#include <boost/thread.hpp>

#include <stdint.h>

#include <scroom/blockallocator.hh>
#include <scroom/utilities.hh>

class ThreadPool;

namespace Scroom
{
  namespace MemoryBlobs
  {
    namespace RawPageData
    {
      typedef boost::shared_ptr<uint8_t> Ptr;
      typedef boost::shared_ptr<const uint8_t> ConstPtr;
      typedef boost::weak_ptr<uint8_t> WeakPtr;
    }

    namespace Page
    {
      typedef boost::shared_ptr<Scroom::MemoryBlocks::Page> Ptr;
    }
    typedef std::list<Page::Ptr> PageList;

    class PageProvider : virtual public Scroom::Utils::Base
    {
    public:
      typedef boost::shared_ptr<PageProvider> Ptr;

    private:
      size_t blockCount;
      size_t blockSize;
      Scroom::MemoryBlocks::BlockFactoryInterface::Ptr blockFactoryInterface;
      Scroom::MemoryBlocks::PageList allPages;
      std::list<Scroom::MemoryBlocks::Page*> freePages;
      boost::mutex mut;

    private:
      class MarkPageFree
      {
      private:
        PageProvider::Ptr provider;

      public:
        MarkPageFree(PageProvider::Ptr provider);
        void operator()(Scroom::MemoryBlocks::Page* page);
      };

      friend class MarkPageFree;

    private:
      PageProvider(size_t blockCount, size_t blockSize);

      void markPageFree(Scroom::MemoryBlocks::Page* page);

    public:
      static Ptr create(size_t blockCount, size_t blockSize);
      Page::Ptr getFreePage();
      size_t getPageSize();
    };

    class Blob : virtual public Scroom::Utils::Base
    {
    public:
      typedef boost::shared_ptr<Blob> Ptr;

    private:
      enum State
        {
          UNINITIALIZED,
          CLEAN,
          DIRTY,
          COMPRESSING
        };

      class UnloadData
      {
      private:
        Blob::Ptr blob;

      public:
        UnloadData(Blob::Ptr blob);
        void operator()(uint8_t* data);
      };

      friend class UnloadData;

    private:
      PageProvider::Ptr provider;
      size_t size;
      uint8_t* data;
      State state;
      boost::mutex mut;
      RawPageData::WeakPtr weakData;
      PageList pages;
      boost::shared_ptr<ThreadPool> cpuBound;
      int refcount; // Yuk

    private:
      Blob(PageProvider::Ptr provider, size_t size);
      void unload();
      RawPageData::Ptr load();
      void compress();

    public:
      ~Blob();

      static Ptr create(PageProvider::Ptr provider, size_t size);
      RawPageData::Ptr get();
      RawPageData::ConstPtr getConst();
      RawPageData::Ptr initialize(uint8_t value);
    };

    ////////////////////////////////////////////////////////////////////////
    // Implementation

    inline PageProvider::MarkPageFree::MarkPageFree(PageProvider::Ptr provider_)
      : provider(provider_)
    {}

    inline void PageProvider::MarkPageFree::operator()(Scroom::MemoryBlocks::Page* p)
    { provider->markPageFree(p); }

    ////////////////////////////////////////////////////////////////////////

    inline Blob::UnloadData::UnloadData(Blob::Ptr blob_)
      : blob(blob_)
    {}

    inline void Blob::UnloadData::operator()(uint8_t*)
    {
      blob->unload();
      blob.reset();
    }
  }
}

