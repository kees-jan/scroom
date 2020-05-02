/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "blob-compression.hh"

#include <stdio.h>
#include <zlib.h>

namespace Scroom
{
  namespace MemoryBlobs
  {
    namespace Detail
    {
      PageList compressBlob(const uint8_t* in, size_t size, PageProvider::Ptr provider)
      {
        PageList result;
        z_stream stream;
        const size_t pageSize = provider->getPageSize();

        stream.next_in = const_cast<uint8_t*>(in);
        stream.avail_in = static_cast<unsigned int>(size);
        stream.avail_out = 0;
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;

        int r = deflateInit(&stream, Z_BEST_SPEED);
        if(r != Z_OK)
        {
          printf("PANIC: deflateInit said: %d (%s)\n",
                 r, (stream.msg?stream.msg:""));
          exit(-1);
        }

        do
        {
          if(stream.avail_out != 0)
            printf("PANIC! Some space available after compression finishes\n");

          Page::Ptr currentPage = provider->getFreePage();
          result.push_back(currentPage);

          RawPageData::Ptr currentPageRaw = currentPage->get();

          stream.next_out = currentPageRaw.get();
          stream.avail_out = static_cast<unsigned int>(pageSize);

          r = deflate(&stream, Z_FINISH);

        } while(r==Z_OK);

        if(r!=Z_STREAM_END)
        {
          printf("PANIC: deflate said: %d (%s)\n",
                 r, (stream.msg?stream.msg:""));
          exit(-1);
        }

        r = deflateEnd(&stream);
        if(r!=Z_OK)
        {
          printf("PANIC: deflateEnd said: %d (%s)\n",
                 r, (stream.msg?stream.msg:""));
          exit(-1);
        }

        // printf("Compression finished: Before: %ld, After: %ld (%lu*%lu=%lu) (%.2f%%, %.2f%%)\n",
        //        stream.total_in, stream.total_out, result.size(), pageSize, result.size()*pageSize,
        //        100.0*stream.total_out/stream.total_in,
        //        100.0*result.size()*pageSize/stream.total_in);

        return result;
      }

      void decompressBlob(uint8_t* out, size_t size, PageList list, PageProvider::Ptr provider)
      {
        z_stream stream;
        const size_t pageSize = provider->getPageSize();

        stream.next_out = out;
        stream.avail_out = static_cast<unsigned int>(size);
        stream.avail_in = 0;
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;

        int r = inflateInit(&stream);
        if(r != Z_OK)
        {
          printf("PANIC: inflateInit said: %d (%s)\n",
                 r, (stream.msg?stream.msg:""));
          exit(-1);
        }

        while(!list.empty() && r== Z_OK)
        {
          if(stream.avail_in != 0)
            printf("PANIC! Some data available after inflation finishes\n");

          Page::Ptr currentPage = list.front();
          list.pop_front();

          RawPageData::Ptr currentPageRaw = currentPage->get();

          stream.next_in = currentPageRaw.get();
          stream.avail_in = static_cast<unsigned int>(pageSize);

          int flush = (list.empty()?Z_FINISH:Z_NO_FLUSH);

          r = inflate(&stream, flush);
        }
        if(r!=Z_OK && r!=Z_STREAM_END)
        {
          printf("PANIC: inflate said: %d (%s)\n",
                 r, (stream.msg?stream.msg:""));
          exit(-1);
        }

        r = inflateEnd(&stream);
        if(r!=Z_OK)
        {
          printf("PANIC: inflateEnd said: %d (%s)\n",
                 r, (stream.msg?stream.msg:""));
          exit(-1);
        }
      }
    }
  }
}

