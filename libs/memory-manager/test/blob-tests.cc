/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstring>
#include <list>

#include <boost/test/unit_test.hpp>

#include <scroom/memoryblobs.hh>

//////////////////////////////////////////////////////////////

using namespace Scroom::MemoryBlobs;

BOOST_AUTO_TEST_SUITE(Blob_Tests)

BOOST_AUTO_TEST_CASE(blobs_retain_their_data)
{
  const size_t blobSize   = 16 * 1024;
  const size_t blobCount  = 16;
  const size_t blockCount = 16;
  const size_t blockSize  = 64;

  PageProvider::Ptr provider = PageProvider::create(blockCount, blockSize);

  std::list<Blob::Ptr> blobList;

  for(size_t i = 0; i < blobCount; i++)
  {
    blobList.push_back(Blob::create(provider, blobSize));
  }
  provider.reset();

  uint8_t data = 0;
  for(const Blob::Ptr& b: blobList)
  {
    RawPageData::Ptr raw = b->get();
    BOOST_REQUIRE(raw.get());

    memset(raw.get(), data, blobSize);
    data++;
  }

  data = 0;
  uint8_t expected[blobSize];
  for(const Blob::Ptr& b: blobList)
  {
    RawPageData::ConstPtr raw = b->getConst();
    BOOST_REQUIRE(raw.get());

    memset(expected, data, blobSize);
    BOOST_CHECK(!memcmp(expected, raw.get(), blobSize));
    data++;
  }
}

BOOST_AUTO_TEST_CASE(blobs_can_be_updated)
{
  const size_t blobSize   = 16 * 1024;
  const size_t blobCount  = 16;
  const size_t blockCount = 16;
  const size_t blockSize  = 64;

  PageProvider::Ptr provider = PageProvider::create(blockCount, blockSize);

  std::list<Blob::Ptr> blobList;

  for(size_t i = 0; i < blobCount; i++)
  {
    blobList.push_back(Blob::create(provider, blobSize));
  }
  provider.reset();

  uint8_t data = 0;
  for(const Blob::Ptr& b: blobList)
  {
    RawPageData::Ptr raw = b->get();
    BOOST_REQUIRE(raw.get());

    memset(raw.get(), data, blobSize);
    data++;
  }

  data = 0;
  for(const Blob::Ptr& b: blobList)
  {
    RawPageData::Ptr raw = b->get();
    BOOST_REQUIRE(raw.get());

    memset(raw.get(), 255 - data, blobSize);
    data++;
  }

  data = 0;
  uint8_t expected[blobSize];
  for(const Blob::Ptr& b: blobList)
  {
    RawPageData::ConstPtr raw = b->getConst();
    BOOST_REQUIRE(raw.get());

    memset(expected, 255 - data, blobSize);
    BOOST_CHECK(!memcmp(expected, raw.get(), blobSize));
    data++;
  }
}

BOOST_AUTO_TEST_CASE(blobs_can_be_initialized)
{
  const size_t  blobSize   = 4096;
  const size_t  blockCount = 16;
  const size_t  blockSize  = 64;
  const uint8_t value      = 25;

  PageProvider::Ptr provider = PageProvider::create(blockCount, blockSize);

  Blob::Ptr b = Blob::create(provider, blobSize);
  provider.reset();

  RawPageData::Ptr raw = b->initialize(value);

  uint8_t expected[blobSize];
  memset(expected, value, blobSize);

  BOOST_CHECK(!memcmp(expected, raw.get(), blobSize));
}

BOOST_AUTO_TEST_SUITE_END()
