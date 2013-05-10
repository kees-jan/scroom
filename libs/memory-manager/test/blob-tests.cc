/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2013 Kees-Jan Dijkzeul
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
#include <scroom/memoryblobs.hh>

#include <list>

#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>

#include <string.h>

//////////////////////////////////////////////////////////////

using namespace Scroom::MemoryBlobs;

BOOST_AUTO_TEST_SUITE(Blob_Tests)

BOOST_AUTO_TEST_CASE(blobs_retain_their_data)
{
  const size_t blobSize = 16*1024;
  const size_t blobCount = 16;
  const size_t blockCount = 16;
  const size_t blockSize = 64;

  PageProvider::Ptr provider = PageProvider::create(blockCount, blockSize);
  
  std::list<Blob::Ptr> blobList;

  for(size_t i=0; i<blobCount; i++)
  {
    blobList.push_back(Blob::create(provider, blobSize));
  }
  provider.reset();

  uint8_t data = 0;
  BOOST_FOREACH(Blob::Ptr b, blobList)
  {
    RawPageData::Ptr raw = b->get();
    BOOST_REQUIRE(raw.get());

    memset(raw.get(), data, blobSize);
    data++;
  }

  data=0;
  uint8_t expected[blobSize];
  BOOST_FOREACH(Blob::Ptr b, blobList)
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
  const size_t blobSize = 16*1024;
  const size_t blobCount = 16;
  const size_t blockCount = 16;
  const size_t blockSize = 64;

  PageProvider::Ptr provider = PageProvider::create(blockCount, blockSize);
  
  std::list<Blob::Ptr> blobList;

  for(size_t i=0; i<blobCount; i++)
  {
    blobList.push_back(Blob::create(provider, blobSize));
  }
  provider.reset();

  uint8_t data = 0;
  BOOST_FOREACH(Blob::Ptr b, blobList)
  {
    RawPageData::Ptr raw = b->get();
    BOOST_REQUIRE(raw.get());

    memset(raw.get(), data, blobSize);
    data++;
  }

  data = 0;
  BOOST_FOREACH(Blob::Ptr b, blobList)
  {
    RawPageData::Ptr raw = b->get();
    BOOST_REQUIRE(raw.get());

    memset(raw.get(), 255-data, blobSize);
    data++;
  }

  data=0;
  uint8_t expected[blobSize];
  BOOST_FOREACH(Blob::Ptr b, blobList)
  {
    RawPageData::ConstPtr raw = b->getConst();
    BOOST_REQUIRE(raw.get());

    memset(expected, 255-data, blobSize);
    BOOST_CHECK(!memcmp(expected, raw.get(), blobSize));
    data++;
  }
}

BOOST_AUTO_TEST_CASE(blobs_can_be_initialized)
{
  const size_t blobSize = 4096;
  const size_t blockCount = 16;
  const size_t blockSize = 64;
  const uint8_t value = 25;

  PageProvider::Ptr provider = PageProvider::create(blockCount, blockSize);

  Blob::Ptr b = Blob::create(provider, blobSize);
  provider.reset();

  RawPageData::Ptr raw = b->initialize(value);

  uint8_t expected[blobSize];
  memset(expected, value, blobSize);

  BOOST_CHECK(!memcmp(expected, raw.get(), blobSize));
}

BOOST_AUTO_TEST_SUITE_END()
