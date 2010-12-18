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

#include <stdio.h>

#include <map>

#include <boost/thread.hpp>

#include <scroom/memorymanagerinterface.hh>
#include <scroom/threadpool.hh>

const std::string SCROOM_MEMORY_HWM = "SCROOM_MEMORY_HWM";
const std::string SCROOM_MEMORY_LWM = "SCROOM_MEMORY_LWM";
const std::string SCROOM_FILES_HWM = "SCROOM_FILES_HWM";
const std::string SCROOM_FILES_LWM = "SCROOM_FILES_LWM";

struct ManagedInfo
{
  size_t size;
  int fdcount;
  bool isLoaded;
  unsigned long timestamp;

public:
  ManagedInfo(size_t size=0, int fdcount=0)
    : size(size), fdcount(fdcount), isLoaded(false), timestamp(0)
  {
  }
};

class MemoryManager
{
public:
  typedef std::map<MemoryManagedInterface*,ManagedInfo> ManagedInfoMap;
  
private:
  ThreadPool thread;

  unsigned long long memHwm;
  unsigned long long memLwm;
  unsigned long long filesHwm;
  unsigned long long filesLwm;

  unsigned long long memCurrent;
  unsigned long long memTotal;
  unsigned long long filesCurrent;
  unsigned long long filesTotal;
  boost::mutex currentMut;

  unsigned long long timestamp;

  boost::mutex mut;
  ManagedInfoMap managedInfo;
  bool isGarbageCollecting;

  unsigned long long unloaders;
  boost::condition_variable unloadersCond;
  boost::mutex unloadersMut;

  
public:
  static MemoryManager* instance();

public:
  MemoryManager();
  ~MemoryManager();

  void registerMMI(MemoryManagedInterface* object, size_t size, int fdcount);
  void unregisterMMI(MemoryManagedInterface* object);
  void loadNotification(MemoryManagedInterface* object);
  void unloadNotification(MemoryManagedInterface* object);

  void garbageCollect();
  void unload(MemoryManagedInterface* i);
  
private:
  int fetchFromEnvironment(std::string v);
  void checkForOutOfResources();
};

class GarbageCollector : public WorkInterface
{
public:
  virtual bool doWork()
  {
    MemoryManager::instance()->garbageCollect();
    return false;
  }
};

class Unloader : public WorkInterface
{
private:
  MemoryManagedInterface* i;
public:
  Unloader(MemoryManagedInterface* i)
    : i(i)
  {}

  virtual bool doWork()
  {
    MemoryManager::instance()->unload(i);
    return false;
  }
};

////////////////////////////////////////////////////////////////////////
/// MemoryManager

MemoryManager* MemoryManager::instance()
{
  static MemoryManager m;
  return &m;
}

MemoryManager::MemoryManager()
  : thread(1)
{
  printf("Creating memory manager\n");

  memHwm = 1024*1024*(unsigned long long)fetchFromEnvironment(SCROOM_MEMORY_HWM);
  memLwm = 1024*1024*(unsigned long long)fetchFromEnvironment(SCROOM_MEMORY_LWM);
  filesHwm = fetchFromEnvironment(SCROOM_FILES_HWM);
  filesLwm = fetchFromEnvironment(SCROOM_FILES_LWM);

  if(!memHwm)
  {
    memHwm = (unsigned long long)2048*1024*1024;
    memLwm = (unsigned long long)1920*1024*1024;
  }
  if(!memLwm)
  {
    memLwm = memHwm*7/8;
  }
  if(!filesHwm)
  {
    filesHwm = 768;
    filesLwm = 700;
  }
  if(!filesLwm)
  {
    filesLwm = filesHwm*8/10;
  }

  printf("Memory watermarks: High %llu, Low %llu\n", memHwm, memLwm);
  printf("File watermarks: High %llu, Low %llu\n", filesHwm, filesLwm);

  memCurrent = 0;
  memTotal = 0;
  filesCurrent = 0;
  filesTotal = 0;

  timestamp = 0;
}

MemoryManager::~MemoryManager()
{
}

void MemoryManager::registerMMI(MemoryManagedInterface* object, size_t size, int fdcount)
{
  boost::unique_lock<boost::mutex> lock(mut);
  managedInfo[object] = ManagedInfo(size, fdcount);
  memTotal+=size;
  filesTotal+=fdcount;
}

void MemoryManager::unregisterMMI(MemoryManagedInterface* object)
{
  boost::unique_lock<boost::mutex> lock(mut);
  ManagedInfoMap::iterator info = managedInfo.find(object);
  if(info != managedInfo.end())
  {
    ManagedInfo& m = info->second;
    if(m.isLoaded)
    {
      boost::unique_lock<boost::mutex> lock(currentMut);
      memCurrent-=m.size;
      filesCurrent-=m.fdcount;
      m.isLoaded = false;
    }
    
    memTotal-=m.size;
    filesTotal-=m.fdcount;
    managedInfo.erase(info);
  }
}

void MemoryManager::loadNotification(MemoryManagedInterface* object)
{
  ManagedInfo& m = managedInfo[object];
  assert(m.size!=0 || m.fdcount!=0);

  if(!m.isLoaded)
  {
    m.isLoaded = true;
    boost::unique_lock<boost::mutex> lock(currentMut);
    memCurrent += m.size;
    filesCurrent += m.fdcount;
  }
  m.timestamp = ++timestamp;

  checkForOutOfResources();
}

void MemoryManager::unloadNotification(MemoryManagedInterface* object)
{
  ManagedInfo& m = managedInfo[object];
  assert(m.size!=0 || m.fdcount!=0);

  if(m.isLoaded)
  {
    m.isLoaded = false;
    boost::unique_lock<boost::mutex> lock(currentMut);
    memCurrent -= m.size;
    filesCurrent -= m.fdcount;
  }
}

int MemoryManager::fetchFromEnvironment(std::string v)
{
  std::string value = getenv(v.c_str());
  return atoi(value.c_str());
}

void MemoryManager::checkForOutOfResources()
{
  if(!isGarbageCollecting && (memCurrent>memHwm || filesCurrent>filesHwm))
  {
    isGarbageCollecting=true;
    thread.schedule(PRIO_NORMAL, new GarbageCollector());
  }
}

void MemoryManager::garbageCollect()
{
  printf("+Garbage collector activating. (%llu/%llu files, %llu/%llu MB)\n",
         filesCurrent, filesTotal, memCurrent/1024/1024, memTotal/1024/1024);

  {
    boost::unique_lock<boost::mutex> lock(mut);
    std::map<unsigned long long, std::list<MemoryManagedInterface*> > sortedInterfaces;

    for(ManagedInfoMap::iterator cur = managedInfo.begin();
        cur != managedInfo.end();
        ++cur)
    {
      sortedInterfaces[cur->second.timestamp].push_back(cur->first);
    }
    std::map<unsigned long long, std::list<MemoryManagedInterface*> >::iterator cur = sortedInterfaces.begin();
    std::map<unsigned long long, std::list<MemoryManagedInterface*> >::iterator end = sortedInterfaces.end();
    unloaders = 0;
    unsigned long long filesExpected = filesCurrent;
    unsigned long long memExpected = memCurrent;

    for(;cur!=end && (filesExpected > filesLwm || memExpected > memHwm); ++cur)
    {
      std::list<MemoryManagedInterface*>& list = cur->second;
      for(std::list<MemoryManagedInterface*>::iterator c = list.begin();
          c!=list.end();
          ++c)
      {
        ManagedInfo& mi = managedInfo[*c];
        
        if(mi.isLoaded)
        {
          boost::unique_lock<boost::mutex> lock(unloadersMut);
          unloaders++;
          schedule(new Unloader(*c), PRIO_HIGHEST);
          filesExpected -= mi.fdcount;
          memExpected -= mi.size;
        }
      }
    }

    {
      printf("=Garbage collector waiting. Expecting %llu files, %llu MB)\n",
             filesExpected, memExpected/1024/1024);
      
      // Wait for unloaders to finish
      boost::unique_lock<boost::mutex> lock(unloadersMut);
      while(unloaders>0)
      {
        unloadersCond.wait(lock);
      }
    }

    isGarbageCollecting = false;
  }

  printf("-Garbage collector terminating. (%llu/%llu files, %llu/%llu MB)\n",
         filesCurrent, filesTotal, memCurrent/1024/1024, memTotal/1024/1024);
}
  
void MemoryManager::unload(MemoryManagedInterface* i)
{
  i->do_unload();
  boost::unique_lock<boost::mutex> lock(unloadersMut);
  unloaders--;
  unloadersCond.notify_all();
}


////////////////////////////////////////////////////////////////////////
/// Regular functions

void registerMMI(MemoryManagedInterface* object, size_t size, int fdcount)
{
  MemoryManager::instance()->registerMMI(object, size, fdcount);
}

void unregisterMMI(MemoryManagedInterface* object)
{
  MemoryManager::instance()->unregisterMMI(object);
}

void loadNotification(MemoryManagedInterface* object)
{
  MemoryManager::instance()->loadNotification(object);
}

void unloadNotification(MemoryManagedInterface* object)
{
  MemoryManager::instance()->unloadNotification(object);
}
