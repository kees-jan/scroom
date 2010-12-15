/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

#ifndef _MEMORYMANAGERINTERFACE_HH
#define _MEMORYMANAGERINTERFACE_HH

#include <stdlib.h>

#include <string>

#include <scroom/global.hh>

class FileBackedMemory
{
private:
  enum State
    {
      UNINITIALIZED,
      UNLOADED,
      LOADED
    };

private:
  State state;
  byte* data;
  size_t size;
  std::string filename;
  bool fileCreated;
  int fd;
  
public:
  FileBackedMemory(size_t size, byte* data=NULL);
  ~FileBackedMemory();

  void initialize(byte value);
  byte* load();
  void unload();
};

class MemoryManagedInterface
{
public:
  virtual ~MemoryManagedInterface()
  {}
  
  virtual bool do_unload()=0;
};

void registerMMI(MemoryManagedInterface* object, size_t size, int fdcount);
void unregisterMMI(MemoryManagedInterface* object);
void loadNotification(MemoryManagedInterface* object);
void unloadNotification(MemoryManagedInterface* object);
  

#endif
