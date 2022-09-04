/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include <boost/move/make_unique.hpp>
#include <boost/move/unique_ptr.hpp>

#include <scroom/blockallocator.hh>
#include <scroom/utilities.hh>

namespace Scroom::MemoryBlocks
{
  namespace Detail
  {
    namespace
    {
      template <typename T>
      class DontDelete
      {
      public:
        void operator()(T*) {}
      };
    } // namespace

    class SwapBasedBlockAllocator
      : public BlockInterface
      , public virtual Scroom::Utils::Base
    {
    private:
      size_t                                count;
      size_t                                size;
      boost::movelib::unique_ptr<uint8_t[]> data;

    private:
      SwapBasedBlockAllocator(size_t count, size_t size);

    protected:
      RawPageData::Ptr get(size_t id) override;

    public:
      static BlockInterface::Ptr create(size_t count, size_t size);
      PageList                   getPages() override;
    };

    SwapBasedBlockAllocator::SwapBasedBlockAllocator(size_t count_, size_t size_)
      : count(count_)
      , size(size_)
      , data(boost::movelib::make_unique<uint8_t[]>(count_ * size_))
    {
    }

    RawPageData::Ptr SwapBasedBlockAllocator::get(size_t id)
    {
      if(id >= count)
      {
        throw std::out_of_range("");
      }

      return RawPageData::Ptr(shared_from_this<SwapBasedBlockAllocator>(), data.get() + id * size);
    }

    BlockInterface::Ptr SwapBasedBlockAllocator::create(size_t count, size_t size)
    {
      return BlockInterface::Ptr(new SwapBasedBlockAllocator(count, size));
    }

    PageList SwapBasedBlockAllocator::getPages()
    {
      BlockInterface::Ptr const me = shared_from_this<BlockInterface>();

      PageList result;
      for(size_t i = 0; i < count; i++)
      {
        result.emplace_back(me, i);
      }

      return result;
    }

    ////////////////////////////////////////////////////////////////////////

    class SwapBasedBlockAllocatorFactory : public BlockFactoryInterface
    {
    private:
      SwapBasedBlockAllocatorFactory() = default;

    public:
      static Ptr          create();
      BlockInterface::Ptr create(size_t count, size_t size) override;
    };

    BlockFactoryInterface::Ptr SwapBasedBlockAllocatorFactory::create()
    {
      return BlockFactoryInterface::Ptr(new SwapBasedBlockAllocatorFactory());
    }

    BlockInterface::Ptr SwapBasedBlockAllocatorFactory::create(size_t count, size_t size)
    {
      return SwapBasedBlockAllocator::create(count, size);
    }
  } // namespace Detail

  ////////////////////////////////////////////////////////////////////////

  BlockFactoryInterface::Ptr getBlockFactoryInterface()
  {
    static BlockFactoryInterface::Ptr const instance = Detail::SwapBasedBlockAllocatorFactory::create();
    return instance;
  }
} // namespace Scroom::MemoryBlocks
