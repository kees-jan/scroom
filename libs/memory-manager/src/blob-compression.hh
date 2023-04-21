/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include <scroom/memoryblobs.hh>

namespace Scroom::MemoryBlobs::Detail
{
  PageList compressBlob(const uint8_t* in, size_t size, const PageProvider::Ptr& provider);
  void     decompressBlob(uint8_t* out, size_t size, PageList list, const PageProvider::Ptr& provider);
} // namespace Scroom::MemoryBlobs::Detail
