/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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

#include <scroom/memorymanagerinterface.hh>

#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#include <zlib.h>

#define TEMPLATE "/tmp/scroom-XXXXXX"
#define BUFSIZE  (strlen(TEMPLATE)+1)

FileBackedMemory::FileBackedMemory(size_t size, byte* data)
  :state(data?LOADED:UNINITIALIZED), data(data), size(size), filename(), fileCreated(false), fd(-1)
{
  long pageSize = sysconf(_SC_PAGE_SIZE);
  size = ((size + pageSize - 1) / pageSize) * pageSize; // round up
}

byte* FileBackedMemory::load()
{
  switch(state)
  {
  case UNINITIALIZED:
    data = new byte[size];
    state = LOADED;
    break;
  case UNLOADED:
    {
      assert(fileCreated);
      data = new byte[size];
      gzFile f = gzopen(filename.c_str(), "r");
      int r = gzread(f, data, size);
      assert(r == (int)size);
      gzclose(f);
      state = LOADED;
    }
    break;
  case LOADED:
  default:
    break;
  }
  return data;
}

void FileBackedMemory::unload()
{
  if(state == LOADED)
  {
    if(!fileCreated)
    {
      // Write data to a temporary file
      char buffer[BUFSIZE];
      strncpy(buffer, TEMPLATE, BUFSIZE);

      fd = mkstemp(buffer);
      assert(fd>=0);
      filename = buffer;
      fileCreated = true;

      gzFile f = gzdopen(fd, "w");
      if(f)
      {
        int remaining = size;
        int offset = 0;
        while(remaining != 0)
        {
          int r = gzwrite(f, data+offset, remaining);
          assert(r<=remaining);
          if(r <= 0)
          {
            // error
            int errorCode = 0;
            const char* errorString = gzerror(f, &errorCode);
            printf("PANIC: An error occurred: (%d, %d, %d) %d, %s\n",
                   offset, remaining, r, 
                   errorCode, errorString);
            abort();
          }
          else
          {
            remaining -= r;
            offset += r;
          }
        }
        gzclose(f);
      }
      else
      {
        int errorCode = 0;
        const char* errorString = gzerror(f, &errorCode);
        printf("PANIC: Open failed: %d, %s\n",
               errorCode, errorString);
        close(fd);
        abort();
      }
    }
    delete[] data;
    data = NULL;
    state = UNLOADED;
  }
}

FileBackedMemory::~FileBackedMemory()
{
  if(data)
    unload();

  if(fileCreated)
  {
    int result = unlink(filename.c_str());
    assert(result>=0);
  }
}

void FileBackedMemory::initialize(byte value)
{
  byte* b = load();
  memset(b, value, size);
}
