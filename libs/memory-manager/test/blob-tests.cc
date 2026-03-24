/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstring>
#include <list>

#include <gtest/gtest.h>

#include <scroom/memoryblobs.hh>

//////////////////////////////////////////////////////////////

using namespace Scroom::MemoryBlobs;

TEST(Blob_Tests, blobs_retain_their_data) // NOLINT
{
  const size_t blobSize = 16 * 1024;
  const size_t blobCount = 16;
  const size_t blockCount = 16;
  const size_t blockSize = 64;

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
    RawPageData::Ptr const raw = b->get();
    ASSERT_TRUE(raw.get());

    memset(raw.get(), data, blobSize);
    data++;
  }

  data = 0;
  uint8_t expected[blobSize];
  for(const Blob::Ptr& b: blobList)
  {
    RawPageData::ConstPtr const raw = b->getConst();
    ASSERT_TRUE(raw.get());

    memset(expected, data, blobSize);
    EXPECT_TRUE(!memcmp(expected, raw.get(), blobSize));
    data++;
  }
}

TEST(Blob_Tests, blobs_can_be_updated) // NOLINT
{
  const size_t blobSize = 16 * 1024;
  const size_t blobCount = 16;
  const size_t blockCount = 16;
  const size_t blockSize = 64;

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
    RawPageData::Ptr const raw = b->get();
    ASSERT_TRUE(raw.get());

    memset(raw.get(), data, blobSize);
    data++;
  }

  data = 0;
  for(const Blob::Ptr& b: blobList)
  {
    RawPageData::Ptr const raw = b->get();
    ASSERT_TRUE(raw.get());

    memset(raw.get(), 255 - data, blobSize);
    data++;
  }

  data = 0;
  uint8_t expected[blobSize];
  for(const Blob::Ptr& b: blobList)
  {
    RawPageData::ConstPtr const raw = b->getConst();
    ASSERT_TRUE(raw.get());

    memset(expected, 255 - data, blobSize);
    EXPECT_TRUE(!memcmp(expected, raw.get(), blobSize));
    data++;
  }
}

TEST(Blob_Tests, blobs_can_be_initialized) // NOLINT
{
  const size_t blobSize = 4096;
  const size_t blockCount = 16;
  const size_t blockSize = 64;
  const uint8_t value = 25;

  PageProvider::Ptr provider = PageProvider::create(blockCount, blockSize);

  Blob::Ptr const b = Blob::create(provider, blobSize);
  provider.reset();

  RawPageData::Ptr const raw = b->initialize(value);

  uint8_t expected[blobSize];
  memset(expected, value, blobSize);

  EXPECT_TRUE(!memcmp(expected, raw.get(), blobSize));
}
