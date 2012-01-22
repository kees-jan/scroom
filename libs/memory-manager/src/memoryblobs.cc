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

#include <boost/foreach.hpp>

using namespace Scroom::MemoryBlocks;

namespace Scroom
{
  namespace MemoryBlobs
  {
    namespace
    {
      template<typename T>
      class DontDelete
      {
      public:
        void operator()(T*) {}
      };
    }

    ////////////////////////////////////////////////////////////////////////

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
      : provider(provider), size(size), data((uint8_t*)malloc(size*sizeof(uint8_t)))
    {
    }

    Blob::~Blob()
    {
      free(data);
    }

    RawPageData::Ptr Blob::get()
    {
      return RawPageData::Ptr(data, DontDelete<uint8_t>());
    }

    RawPageData::ConstPtr Blob::getConst() const
    {
      return RawPageData::ConstPtr(data, DontDelete<uint8_t>());
    }
  }
}

