/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstring>

#include <gtest/gtest.h>

#include "blob-compression.hh"

//////////////////////////////////////////////////////////////

using namespace Scroom::MemoryBlobs;
using namespace Scroom::MemoryBlobs::Detail;

TEST(Blob_Compression_Tests, compression_decompression_retains_data) // NOLINT
{
  const size_t blobSize = 16 * 1024;
  const size_t blockCount = 16;
  const size_t blockSize = 64;

  uint8_t in[blobSize];
  for(size_t i = 0; i < blobSize; i++)
  {
    in[i] = i / 256 + i % 256;
  }

  PageProvider::Ptr const provider = PageProvider::create(blockCount, blockSize);

  PageList const l = compressBlob(in, blobSize, provider);

  uint8_t out[blobSize];
  decompressBlob(out, blobSize, l, provider);

  EXPECT_TRUE(!memcmp(in, out, blobSize));
}

TEST(Blob_Compression_Tests, compression_decompression_retains_data_with_large_blocks) // NOLINT
{
  const size_t blobSize = 16;
  const size_t blockCount = 16;
  const size_t blockSize = 256;

  uint8_t in[blobSize];
  for(size_t i = 0; i < blobSize; i++)
  {
    in[i] = i / 256 + i % 256;
  }

  PageProvider::Ptr const provider = PageProvider::create(blockCount, blockSize);

  PageList const l = compressBlob(in, blobSize, provider);

  uint8_t out[blobSize];
  decompressBlob(out, blobSize, l, provider);

  EXPECT_TRUE(!memcmp(in, out, blobSize));
}
