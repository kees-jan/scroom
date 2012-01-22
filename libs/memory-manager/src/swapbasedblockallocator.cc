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

#include <scroom/blockallocator.hh>

namespace Scroom
{
  namespace MemoryBlocks
  {
    namespace Detail
    {
      class SwapBasedBlockAllocator : public BlockInterface
      {
      private:
        SwapBasedBlockAllocator(size_t count, size_t size);
        
      protected:
        virtual RawPageData::Ptr get(size_t id);
      
      public:
        static BlockInterface::Ptr create(size_t count, size_t size);
        virtual PageList getPages();
      };

      SwapBasedBlockAllocator::SwapBasedBlockAllocator(size_t count, size_t size)
      {}

      RawPageData::Ptr SwapBasedBlockAllocator::get(size_t id)
      {
        return RawPageData::Ptr();
      }

      BlockInterface::Ptr SwapBasedBlockAllocator::create(size_t count, size_t size)
      {
        return BlockInterface::Ptr(new SwapBasedBlockAllocator(count, size));
      }

      PageList SwapBasedBlockAllocator::getPages()
      {
        return PageList();
      }
      
      ////////////////////////////////////////////////////////////////////////

      class SwapBasedBlockAllocatorFactory : public BlockFactoryInterface
      {
      private:
        SwapBasedBlockAllocatorFactory();
        
      public:
        static Ptr create();
        virtual BlockInterface::Ptr create(size_t count, size_t size);
      };
      
      SwapBasedBlockAllocatorFactory::SwapBasedBlockAllocatorFactory()
      {}
      
      BlockFactoryInterface::Ptr SwapBasedBlockAllocatorFactory::create()
      {
        return BlockFactoryInterface::Ptr(new SwapBasedBlockAllocatorFactory());
      }

      BlockInterface::Ptr SwapBasedBlockAllocatorFactory::create(size_t count, size_t size)
      {
        return SwapBasedBlockAllocator::create(count, size);
      }
    }

    ////////////////////////////////////////////////////////////////////////
    
    BlockFactoryInterface::Ptr getBlockFactoryInterface()
    {
      static BlockFactoryInterface::Ptr instance = Detail::SwapBasedBlockAllocatorFactory::create();
      return instance;
    }
  }
}

