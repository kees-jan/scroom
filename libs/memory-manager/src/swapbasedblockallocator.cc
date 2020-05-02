/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/blockallocator.hh>

#include <scroom/utilities.hh>

namespace Scroom
{
  namespace MemoryBlocks
  {
    namespace Detail
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

      class SwapBasedBlockAllocator : public BlockInterface, public virtual Scroom::Utils::Base
      {
      private:
        size_t count;
        size_t size;
        uint8_t* data;

      private:
        SwapBasedBlockAllocator(size_t count, size_t size);

      protected:
        virtual RawPageData::Ptr get(size_t id);

      public:
        ~SwapBasedBlockAllocator();
        static BlockInterface::Ptr create(size_t count, size_t size);
        virtual PageList getPages();
      };

      SwapBasedBlockAllocator::SwapBasedBlockAllocator(size_t count_, size_t size_)
        : count(count_), size(size_), data(static_cast<uint8_t*>(malloc(count_*size_*sizeof(uint8_t))))
      {
        if(data==NULL)
          throw std::bad_alloc();
      }

      SwapBasedBlockAllocator::~SwapBasedBlockAllocator()
      {
        free(data);
      }

      RawPageData::Ptr SwapBasedBlockAllocator::get(size_t id)
      {
        if(id>=count)
          throw std::out_of_range("");

        return RawPageData::Ptr(data+id*size, DontDelete<uint8_t>());
      }

      BlockInterface::Ptr SwapBasedBlockAllocator::create(size_t count, size_t size)
      {
        return BlockInterface::Ptr(new SwapBasedBlockAllocator(count, size));
      }

      PageList SwapBasedBlockAllocator::getPages()
      {
        BlockInterface::Ptr me = shared_from_this<BlockInterface>();

        PageList result;
        for(size_t i=0; i<count; i++)
          result.push_back(Page(me, i));

        return result;
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

