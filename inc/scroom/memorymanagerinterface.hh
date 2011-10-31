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

#ifndef _MEMORYMANAGERINTERFACE_HH
#define _MEMORYMANAGERINTERFACE_HH

#include <stdlib.h>

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/global.hh>
#include <scroom/registration.hh>

/**
 * Block of memory that is backed-up in a file
 */
class FileBackedMemory
{
private:

  /** State of the memory */
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
  /**
   * Create a FileBackedMemory of the given size
   *
   * If you also pass some @c data, then the data will be deleted at
   * some later point (during unload)
   */
  FileBackedMemory(size_t size, byte* data=NULL);

  /** Destructor */
  ~FileBackedMemory();

  /** Initialize the memory with the given value */
  void initialize(byte value);

  /** Get a pointer to the data, loading it if necessary */
  byte* load();

  /** Unload the data */
  void unload();
};

/**
 * Interface for something that can be managed by a MemoryManager
 */
class MemoryManagedInterface
{
public:
  typedef boost::shared_ptr<MemoryManagedInterface> Ptr;
  typedef boost::weak_ptr<MemoryManagedInterface> WeakPtr;
  
public:
  virtual ~MemoryManagedInterface()
  {}

  /**
   * Request that the object unload itself
   *
   * @retval true if the object unloaded
   * @retval false if the object refused
   */
  virtual bool do_unload()=0;
};

namespace MemoryManager
{
  Scroom::Utils::Stuff registerMMI(MemoryManagedInterface::Ptr object, size_t size, int fdcount);
  void loadNotification(Scroom::Utils::Stuff r);
  void unloadNotification(Scroom::Utils::Stuff r);
}



#endif
