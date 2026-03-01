/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstring>

#include <gtest/gtest.h>

#include <scroom/memoryblobs.hh>

//////////////////////////////////////////////////////////////

using namespace Scroom::MemoryBlobs;

TEST(PageProvider_Tests, Provider_provides_any_number_of_independent_blocks_of_a_given_size) // NOLINT
{
  const size_t size      = 256;
  const size_t count     = 16;
  const size_t testCount = 48;

  PageProvider::Ptr provider = PageProvider::create(count, size);
  PageList          pages;

  uint8_t data = 0;
  for(size_t i = 0; i < testCount; i++)
  {
    Page::Ptr const p = provider->getFreePage();
    pages.push_back(p);

    RawPageData::Ptr const raw = p->get();
    ASSERT_TRUE(raw.get());

    memset(raw.get(), data, size);
    data++;
  }

  provider.reset();

  data = 0;
  uint8_t expected[size];
  for(const Page::Ptr& p: pages)
  {
    RawPageData::Ptr const raw = p->get();
    ASSERT_TRUE(raw.get());

    memset(expected, data, size);
    EXPECT_TRUE(!memcmp(expected, raw.get(), size));
    data++;
  }
}
