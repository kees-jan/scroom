/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "blob-compression.hh"

#include <cstdio>
#include <cstdlib>

#include <fmt/format.h>
#include <zlib.h>

#include <scroom/assertions.hh>
#include <scroom/memoryblobs.hh>

#define zlib_verify(condition, function_name, r, stream)                                                    \
  ((condition) ? ((void)0)                                                                                  \
               : Scroom::Utils::Detail::assertionFailed(                                                    \
                 "assertion",                                                                               \
                 fmt::format("{} said: {} ({})", (function_name), (r), ((stream).msg ? (stream).msg : "")), \
                 static_cast<const char*>(__PRETTY_FUNCTION__),                                             \
                 __FILE__,                                                                                  \
                 __LINE__))

namespace Scroom::MemoryBlobs::Detail
{
  PageList compressBlob(const uint8_t* in, size_t size, const PageProvider::Ptr& provider)
  {
    PageList     result;
    z_stream     stream;
    const size_t pageSize = provider->getPageSize();

    stream.next_in   = const_cast<uint8_t*>(in); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    stream.avail_in  = size;
    stream.avail_out = 0;
    stream.zalloc    = Z_NULL;
    stream.zfree     = Z_NULL;
    stream.opaque    = Z_NULL;

    int r = deflateInit(&stream, Z_BEST_SPEED);
    zlib_verify(r == Z_OK, "deflateInit", r, stream);

    do
    {
      verify(stream.avail_out == 0);

      Page::Ptr currentPage = provider->getFreePage();
      result.push_back(currentPage);

      RawPageData::Ptr currentPageRaw = currentPage->get();

      stream.next_out  = currentPageRaw.get();
      stream.avail_out = pageSize;

      r = deflate(&stream, Z_FINISH);

    } while(r == Z_OK);

    zlib_verify(r == Z_STREAM_END, "deflate", r, stream);

    r = deflateEnd(&stream);
    zlib_verify(r == Z_OK, "deflateEnd", r, stream);

    return result;
  }

  void decompressBlob(uint8_t* out, size_t size, PageList list, const PageProvider::Ptr& provider)
  {
    z_stream     stream;
    const size_t pageSize = provider->getPageSize();

    stream.next_out  = out;
    stream.avail_out = size;
    stream.avail_in  = 0;
    stream.zalloc    = Z_NULL;
    stream.zfree     = Z_NULL;
    stream.opaque    = Z_NULL;

    int r = inflateInit(&stream);
    zlib_verify(r == Z_OK, "inflateInit", r, stream);

    while(!list.empty() && r == Z_OK)
    {
      verify(stream.avail_in == 0);

      Page::Ptr currentPage = list.front();
      list.pop_front();

      RawPageData::Ptr currentPageRaw = currentPage->get();

      stream.next_in  = currentPageRaw.get();
      stream.avail_in = pageSize;

      int flush = (list.empty() ? Z_FINISH : Z_NO_FLUSH);

      r = inflate(&stream, flush);
    }
    zlib_verify(r == Z_OK || r == Z_STREAM_END, "inflate", r, stream);

    r = inflateEnd(&stream);
    zlib_verify(r == Z_OK, "inflateEnd", r, stream);
  }
} // namespace Scroom::MemoryBlobs::Detail
