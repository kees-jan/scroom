/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <string.h>

#include <boost/test/unit_test.hpp>

#include <scroom/blockallocator.hh>

//////////////////////////////////////////////////////////////

using namespace Scroom::MemoryBlocks;

BOOST_AUTO_TEST_SUITE(Swap_based_Block_Allocator_Tests)

BOOST_AUTO_TEST_CASE(allocator_provides_a_number_of_independent_blocks_of_a_given_size)
{
  const size_t size  = 16 * 1024;
  const size_t count = 16;

  BlockFactoryInterface::Ptr bfi = getBlockFactoryInterface();
  BlockInterface::Ptr        bi  = bfi->create(count, size);

  PageList pages = bi->getPages();
  BOOST_CHECK_EQUAL(count, pages.size());

  bi.reset();

  uint8_t data = 0;
  for(Page& p: pages)
  {
    RawPageData::Ptr raw = p.get();
    BOOST_REQUIRE(raw.get());

    memset(raw.get(), data, size);
    data++;
  }

  data = 0;
  uint8_t expected[size];
  for(Page& p: pages)
  {
    RawPageData::Ptr raw = p.get();
    BOOST_REQUIRE(raw.get());

    memset(expected, data, size);
    BOOST_CHECK(!memcmp(expected, raw.get(), size));
    data++;
  }
}

BOOST_AUTO_TEST_SUITE_END()
