/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstring>

#include <gtest/gtest.h>

#include <scroom/blockallocator.hh>

//////////////////////////////////////////////////////////////

using namespace Scroom::MemoryBlocks;

TEST(Swap_based_Block_Allocator_Tests, allocator_provides_a_number_of_independent_blocks_of_a_given_size) // NOLINT
{
  const size_t size  = 16 * 1024;
  const size_t count = 16;

  BlockFactoryInterface::Ptr const bfi = getBlockFactoryInterface();
  BlockInterface::Ptr              bi  = bfi->create(count, size);

  PageList pages = bi->getPages();
  EXPECT_EQ(count, pages.size());

  bi.reset();

  uint8_t data = 0;
  for(Page& p: pages)
  {
    RawPageData::Ptr const raw = p.get();
    ASSERT_TRUE(raw.get());

    memset(raw.get(), data, size);
    data++;
  }

  data = 0;
  uint8_t expected[size];
  for(Page& p: pages)
  {
    RawPageData::Ptr const raw = p.get();
    ASSERT_TRUE(raw.get());

    memset(expected, data, size);
    EXPECT_TRUE(!memcmp(expected, raw.get(), size));
    data++;
  }
}
