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

#ifndef BLOCKALLOCATOR_HH
#define BLOCKALLOCATOR_HH

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
      RawPageData::Ptr get();
    };

    typedef std::list<Page> PageList;
    
    class BlockInterface
    {
    public:
      typedef boost::shared_ptr<BlockInterface> Ptr;
      typedef boost::weak_ptr<BlockInterface> WeakPtr;

    private:
      RawPageData::Ptr get(size_t id);
      
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

    inline RawPageData::Ptr Page::get()
    { return bi->get(id); }
  }
}


#endif
