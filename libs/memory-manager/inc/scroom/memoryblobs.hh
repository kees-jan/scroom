/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <utility>

#include <boost/thread.hpp>

#include <scroom/blockallocator.hh>
#include <scroom/utilities.hh>

class ThreadPool;

namespace Scroom::MemoryBlobs
{
  namespace RawPageData
  {
    using Ptr      = std::shared_ptr<uint8_t>;
    using ConstPtr = std::shared_ptr<const uint8_t>;
    using WeakPtr  = std::weak_ptr<uint8_t>;
  } // namespace RawPageData

  namespace Page
  {
    using Ptr = std::shared_ptr<Scroom::MemoryBlocks::Page>;
  }
  using PageList = std::list<Page::Ptr>;

  class PageProvider : virtual public Scroom::Utils::Base
  {
  public:
    using Ptr = std::shared_ptr<PageProvider>;

  private:
    size_t                                           blockCount;
    size_t                                           blockSize;
    Scroom::MemoryBlocks::BlockFactoryInterface::Ptr blockFactoryInterface;
    Scroom::MemoryBlocks::PageList                   allPages;
    std::list<Scroom::MemoryBlocks::Page*>           freePages;
    boost::mutex                                     mut;

  private:
    class MarkPageFree
    {
    private:
      PageProvider::Ptr provider;

    public:
      explicit MarkPageFree(PageProvider::Ptr provider);
      void operator()(Scroom::MemoryBlocks::Page* page);
    };

    friend class MarkPageFree;

  private:
    PageProvider(size_t blockCount, size_t blockSize);

    void markPageFree(Scroom::MemoryBlocks::Page* page);

  public:
    static Ptr create(size_t blockCount, size_t blockSize);
    Page::Ptr  getFreePage();
    size_t     getPageSize() const;
  };

  class Blob : virtual public Scroom::Utils::Base
  {
  public:
    using Ptr = std::shared_ptr<Blob>;

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
      explicit UnloadData(Blob::Ptr blob);
      void operator()(uint8_t* data);
    };

    friend class UnloadData;

  private:
    PageProvider::Ptr           provider;
    size_t                      size;
    uint8_t*                    data{nullptr};
    State                       state{UNINITIALIZED};
    boost::mutex                mut;
    RawPageData::WeakPtr        weakData;
    PageList                    pages;
    std::shared_ptr<ThreadPool> cpuBound;
    int                         refcount{0}; // Yuk

  private:
    Blob(PageProvider::Ptr provider, size_t size);
    void             unload();
    RawPageData::Ptr load();
    void             compress();

  public:
    static Ptr            create(PageProvider::Ptr provider, size_t size);
    RawPageData::Ptr      get();
    RawPageData::ConstPtr getConst();
    RawPageData::Ptr      initialize(uint8_t value);
  };

  ////////////////////////////////////////////////////////////////////////
  // Implementation

  inline PageProvider::MarkPageFree::MarkPageFree(PageProvider::Ptr provider_)
    : provider(std::move(provider_))
  {
  }

  inline void PageProvider::MarkPageFree::operator()(Scroom::MemoryBlocks::Page* p) { provider->markPageFree(p); }

  ////////////////////////////////////////////////////////////////////////

  inline Blob::UnloadData::UnloadData(Blob::Ptr blob_)
    : blob(std::move(blob_))
  {
  }

  inline void Blob::UnloadData::operator()(uint8_t* /*unused*/)
  {
    blob->unload();
    blob.reset();
  }
} // namespace Scroom::MemoryBlobs
