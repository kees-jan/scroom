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

#ifndef MEMORYBLOBS_HH
#define MEMORYBLOBS_HH

#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread.hpp>

#include <stdint.h>

#include <scroom/blockallocator.hh>
#include <scroom/utilities.hh>

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
    };

    class Blob
    {
    public:
      typedef boost::shared_ptr<Blob> Ptr;

    private:
      PageProvider::Ptr provider;
      size_t size;
      uint8_t* data;

    private:
      Blob(PageProvider::Ptr provider, size_t size);

    public:
      ~Blob();
      
      static Ptr create(PageProvider::Ptr provider, size_t size);
      RawPageData::Ptr get();
      RawPageData::ConstPtr getConst() const;
    };

    ////////////////////////////////////////////////////////////////////////
    // Implementation

    inline PageProvider::MarkPageFree::MarkPageFree(PageProvider::Ptr provider)
      : provider(provider)
    {}

    inline void PageProvider::MarkPageFree::operator()(Scroom::MemoryBlocks::Page* p)
    { provider->markPageFree(p); }
  }
}


#endif
