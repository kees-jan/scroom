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
      fd = open(filename.c_str(), O_RDWR);
      assert(fd>=0);

      data = (byte*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

      assert(data != (byte*)-1); // == MAP_FAILED
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
    if(fileCreated)
    {
      // Execute unmap
      int result = munmap(data, size);
      data = NULL;
      assert(result>=0);

      result = close(fd);
      fd=-1;
      assert(result>=0);
    }
    else
    {
      // Write data to a temporary file
      char buffer[BUFSIZE];
      strncpy(buffer, TEMPLATE, BUFSIZE);

      fd = mkstemp(buffer);
      assert(fd>=0);
      filename = buffer;
      fileCreated = true;

      size_t remaining = size;
      byte* current = data;
      ssize_t result=0;
      while(remaining>0)
      {
        result = write(fd, current, remaining);
        assert(result>0);
        remaining -= result;
        current += result;
      }
      close(fd);
      delete[] data;
      data=NULL;
    }
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
