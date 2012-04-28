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

#include <scroom/memoryblobs.hh>

#include <stdio.h>

#include <boost/foreach.hpp>

#include <scroom/threadpool.hh>

#include "blob-compression.hh"

using namespace Scroom::MemoryBlocks;

#define COMPRESS_PRIO PRIO_HIGHER

namespace Scroom
{
  namespace MemoryBlobs
  {
    PageProvider::PageProvider(size_t blockCount, size_t blockSize)
      : blockCount(blockCount), blockSize(blockSize), blockFactoryInterface(getBlockFactoryInterface())
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
        BOOST_FOREACH(Scroom::MemoryBlocks::Page& p, pages)
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

    Blob::Blob(PageProvider::Ptr provider, size_t size)
      : provider(provider), size(size), data(NULL), state(UNINITIALIZED), cpuBound(CpuBound())
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
          data = (uint8_t*)malloc(size*sizeof(uint8_t));
          break;
        case CLEAN:
          // Decompress data
          data = (uint8_t*)malloc(size*sizeof(uint8_t));
          Detail::decompressBlob(data, size, pages, provider);
          break;
        case DIRTY:
          printf("PANIC: load() unexpected in state %d\n", state);
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
      }
      
      return result;
    }
    
    void Blob::unload()
    {
      boost::mutex::scoped_lock lock(mut);
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

    void Blob::compress()
    {
      boost::mutex::scoped_lock lock(mut);
      if(state==COMPRESSING)
      {
        pages.clear();
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
