/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <scroom/memoryblobs.hh>

namespace Scroom
{
  namespace MemoryBlobs
  {
    namespace Detail
    {
      PageList compressBlob(const uint8_t* in, size_t size, PageProvider::Ptr provider);
      void     decompressBlob(uint8_t* out, size_t size, PageList list, PageProvider::Ptr provider);
    } // namespace Detail
  }   // namespace MemoryBlobs
} // namespace Scroom
