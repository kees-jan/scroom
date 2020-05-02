/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <blob-compression.hh>

#include <boost/test/unit_test.hpp>

#include <string.h>

//////////////////////////////////////////////////////////////

using namespace Scroom::MemoryBlobs;
using namespace Scroom::MemoryBlobs::Detail;

BOOST_AUTO_TEST_SUITE(Blob_Compression_Tests)

BOOST_AUTO_TEST_CASE(compression_decompression_retains_data)
{
  const size_t blobSize = 16*1024;
  const size_t blockCount = 16;
  const size_t blockSize = 64;

  uint8_t in[blobSize];
  for(size_t i=0; i<blobSize; i++)
    in[i] = static_cast<uint8_t>(i/256 + i%256);

  PageProvider::Ptr provider = PageProvider::create(blockCount, blockSize);

  PageList l = compressBlob(in, blobSize, provider);

  uint8_t out[blobSize];
  decompressBlob(out, blobSize, l, provider);

  BOOST_CHECK(!memcmp(in, out, blobSize));
}

BOOST_AUTO_TEST_CASE(compression_decompression_retains_data_with_large_blocks)
{
  const size_t blobSize = 16;
  const size_t blockCount = 16;
  const size_t blockSize = 256;

  uint8_t in[blobSize];
  for(size_t i=0; i<blobSize; i++)
    in[i] = static_cast<uint8_t>(i/256 + i%256);

  PageProvider::Ptr provider = PageProvider::create(blockCount, blockSize);

  PageList l = compressBlob(in, blobSize, provider);

  uint8_t out[blobSize];
  decompressBlob(out, blobSize, l, provider);

  BOOST_CHECK(!memcmp(in, out, blobSize));
}

BOOST_AUTO_TEST_SUITE_END()
