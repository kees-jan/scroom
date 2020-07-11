/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/memoryblobs.hh>

#include <stdio.h>

#include <scroom/threadpool.hh>

#include "blob-compression.hh"

using namespace Scroom::MemoryBlocks;

#define COMPRESS_PRIO PRIO_HIGHER

namespace Scroom
{
  namespace MemoryBlobs
  {
    PageProvider::PageProvider(size_t blockCount_, size_t blockSize_)
      : blockCount(blockCount_), blockSize(blockSize_), blockFactoryInterface(getBlockFactoryInterface())
    {}

    PageProvider::Ptr PageProvider::create(size_t blockCount, size_t blockSize)
    {
      return Ptr(new PageProvider(blockCount, blockSize));
    }

    Page::Ptr PageProvider::getFreePage()
    {
      boost::mutex::scoped_lock lock(mut);
      if(freePages.empty())
      {
        Scroom::MemoryBlocks::PageList pages = blockFactoryInterface->create(blockCount, blockSize)->getPages();
        for(Scroom::MemoryBlocks::Page& p: pages)
        {
          freePages.push_back(&p);
        }
        allPages.splice(allPages.end(), pages);
      }

      Page::Ptr result(freePages.front(), MarkPageFree(shared_from_this<PageProvider>()));
      freePages.pop_front();

      return result;
    }

    size_t PageProvider::getPageSize()
    {
      return blockSize;
    }

    void PageProvider::markPageFree(Scroom::MemoryBlocks::Page* p)
    {
      boost::mutex::scoped_lock lock(mut);
      freePages.push_front(p);
    }

    ////////////////////////////////////////////////////////////////////////

    Blob::Ptr Blob::create(PageProvider::Ptr provider, size_t size)
    {
      return Ptr(new Blob(provider, size));
    }

    Blob::Blob(PageProvider::Ptr provider_, size_t size_)
      : provider(provider_), size(size_), data(NULL), state(UNINITIALIZED), cpuBound(CpuBound()), refcount(0)
    {
    }

    Blob::~Blob()
    {
    }

    RawPageData::Ptr Blob::load()
    {
      RawPageData::Ptr result = weakData.lock();
      if(!result)
      {
        switch(state)
        {
        case UNINITIALIZED:
          // Allocate new data
          data = static_cast<uint8_t*>(malloc(size*sizeof(uint8_t)));
          break;
        case CLEAN:
          // Decompress data
          data = static_cast<uint8_t*>(malloc(size*sizeof(uint8_t)));
          Detail::decompressBlob(data, size, pages, provider);
          break;
        case DIRTY:
          break;
        case COMPRESSING:
          // Data is currently being compressed. Abort...
          state = DIRTY;
          break;
        default:
          printf("PANIC: Illegal state %d\n", state);
          break;
        }

        // data should point to something valid here...
        result = RawPageData::Ptr(data, UnloadData(shared_from_this<Blob>()));
        weakData = result;
        refcount++;
      }

      return result;
    }

    void Blob::unload()
    {
      boost::mutex::scoped_lock lock(mut);
      refcount--;
      if(refcount==0)
      {
        if(state==DIRTY)
        {
          state = COMPRESSING;
          cpuBound->schedule(boost::bind(&Blob::compress, shared_from_this<Blob>()), COMPRESS_PRIO);
        }
        else
        {
          free(data);
          data = NULL;
        }
      }
    }

    void Blob::compress()
    {
      boost::mutex::scoped_lock lock(mut);
      if(state==COMPRESSING)
      {
        if(refcount!=0)
          printf("PANIC: Compressing with pending references\n");

        pages = Detail::compressBlob(data, size, provider);
        free(data);
        data = NULL;

        state=CLEAN;
      }
    }

    RawPageData::Ptr Blob::get()
    {
      boost::mutex::scoped_lock lock(mut);
      RawPageData::Ptr result = load();
      state = DIRTY;
      return result;
    }

    RawPageData::Ptr Blob::initialize(uint8_t value)
    {
      boost::mutex::scoped_lock lock(mut);
      RawPageData::Ptr result = load();
      state = DIRTY;
      memset(result.get(), value, size);
      return result;
    }

    RawPageData::ConstPtr Blob::getConst()
    {
      boost::mutex::scoped_lock lock(mut);
      return load();
    }
  }
}

