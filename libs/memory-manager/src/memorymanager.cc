/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2013 Kees-Jan Dijkzeul
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
#include <limits>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <scroom/memorymanagerinterface.hh>
#include <scroom/threadpool.hh>

namespace
{
  const std::string SCROOM_MEMORY_HWM = "SCROOM_MEMORY_HWM";
  const std::string SCROOM_MEMORY_LWM = "SCROOM_MEMORY_LWM";
  const std::string SCROOM_FILES_HWM = "SCROOM_FILES_HWM";
  const std::string SCROOM_FILES_LWM = "SCROOM_FILES_LWM";

  ////////////////////////////////////////////////////////////////////////
  
  struct ManagedInfo
  {
    MemoryManagedInterface::WeakPtr object;
    size_t size;
    int fdcount;
    bool isLoaded;
    unsigned long timestamp;

  public:
    ManagedInfo(MemoryManagedInterface::Ptr object=MemoryManagedInterface::Ptr(),
                size_t size=0, int fdcount=0)
      : object(object), size(size), fdcount(fdcount), isLoaded(false), timestamp(0)
    {
    }
  };

  ////////////////////////////////////////////////////////////////////////

  struct Registration
  {
    typedef boost::shared_ptr<Registration> Ptr;
    typedef boost::weak_ptr<Registration> WeakPtr;
    
    Scroom::Utils::StuffWeak r;

    ~Registration();
    static Scroom::Utils::Stuff create();
  };

  ////////////////////////////////////////////////////////////////////////

  class MemoryManagerImpl
  {
  public:
    typedef std::map<Scroom::Utils::StuffWeak,ManagedInfo> ManagedInfoMap;
    typedef std::pair<Scroom::Utils::StuffWeak,ManagedInfo> ManagedInfoItem;
    typedef boost::shared_ptr<MemoryManagerImpl> Ptr;
  
  private:
    ThreadPool thread;
    bool isGarbageCollecting;

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

    unsigned long long unloaders;
    boost::condition_variable unloadersCond;
    boost::mutex unloadersMut;

  public:
    MemoryManagerImpl();
    ~MemoryManagerImpl();

    Scroom::Utils::Stuff registerMMI(MemoryManagedInterface::Ptr object, size_t size, int fdcount);
    void unregisterMMI(Scroom::Utils::StuffWeak r);
    void loadNotification(Scroom::Utils::Stuff r);
    void unloadNotification(Scroom::Utils::Stuff r);

    void garbageCollect();
    void unload(MemoryManagedInterface::Ptr i);
  
  private:
    int fetchFromEnvironment(std::string v);
    void checkForOutOfResources();
  };

  ////////////////////////////////////////////////////////////////////////
  /// Regular functions

  MemoryManagerImpl::Ptr instance()
  {
    static MemoryManagerImpl::Ptr memoryManager = MemoryManagerImpl::Ptr(new MemoryManagerImpl());
    return memoryManager;
  }

  ////////////////////////////////////////////////////////////////////////
  /// Registration

  Scroom::Utils::Stuff Registration::create()
  {
    Registration::Ptr r = Registration::Ptr(new Registration());
    r->r = r;

    return r;
  }

  Registration::~Registration()
  {
    instance()->unregisterMMI(r);
  }

  ////////////////////////////////////////////////////////////////////////
  /// MemoryManagerImpl

  MemoryManagerImpl::MemoryManagerImpl()
    : thread(1), isGarbageCollecting(false)
  {
    printf("Creating memory manager\n");

    memHwm = 1024*1024*(unsigned long long)fetchFromEnvironment(SCROOM_MEMORY_HWM);
    memLwm = 1024*1024*(unsigned long long)fetchFromEnvironment(SCROOM_MEMORY_LWM);
    filesHwm = fetchFromEnvironment(SCROOM_FILES_HWM);
    filesLwm = fetchFromEnvironment(SCROOM_FILES_LWM);

    if(!memHwm)
    {
      // On 32-bit machines, this is 2Gb, which is a nice enough value
      // On 64-bit machines, this is very large. I hope you have enough swap.
      memHwm = std::numeric_limits<unsigned long>::max()/2;
    }
    if(!memLwm)
    {
      // Order is important. When using the memHwm as computed above, you
      // might be in for an integer overflow
      memLwm = memHwm/8*7;
    }
    if(!filesHwm)
    {
      filesHwm = 800;
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

  MemoryManagerImpl::~MemoryManagerImpl()
  {
  }

  Scroom::Utils::Stuff MemoryManagerImpl::registerMMI(MemoryManagedInterface::Ptr object, size_t size, int fdcount)
  {
    boost::unique_lock<boost::mutex> lock(mut);
    Scroom::Utils::Stuff r = Registration::create();
    managedInfo[r] = ManagedInfo(object, size, fdcount);
    memTotal+=size;
    filesTotal+=fdcount;

    // printf("RegisterMMI:\t+ %llu\t= %llu\n",
    //        (unsigned long long)size/1024/1024, (unsigned long long)memTotal/1024/1024);

    return r;
  }

  void MemoryManagerImpl::unregisterMMI(Scroom::Utils::StuffWeak r)
  {
    boost::unique_lock<boost::mutex> lock(mut);
    ManagedInfoMap::iterator info = managedInfo.find(r);
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

      // printf("UnregisterMMI:\t- %llu\t= %llu\n",
      //        (unsigned long long)m.size/1024/1024, (unsigned long long)memTotal/1024/1024);
    }
  }

  void MemoryManagerImpl::loadNotification(Scroom::Utils::Stuff r)
  {
    ManagedInfo& m = managedInfo[r];
    assert(m.size!=0 || m.fdcount!=0);

    if(!m.isLoaded)
    {
      m.isLoaded = true;
      boost::unique_lock<boost::mutex> lock(currentMut);
      memCurrent += m.size;
      filesCurrent += m.fdcount;

      // printf("LoadNotification:\t+ %d\t= %llu\n",
      //        m.size/1024/1024, (unsigned long long)memCurrent/1024/1024);
    }
    m.timestamp = ++timestamp;

    checkForOutOfResources();
  }

  void MemoryManagerImpl::unloadNotification(Scroom::Utils::Stuff r)
  {
    ManagedInfo& m = managedInfo[r];
    assert(m.size!=0 || m.fdcount!=0);

    if(m.isLoaded)
    {
      m.isLoaded = false;
      boost::unique_lock<boost::mutex> lock(currentMut);
      memCurrent -= m.size;
      filesCurrent -= m.fdcount;

      // printf("UnloadNotification:\t- %d\t= %llu\n",
      //        m.size/1024/1024, (unsigned long long)memCurrent/1024/1024);
    }
  }

  int MemoryManagerImpl::fetchFromEnvironment(std::string v)
  {
    char* val = getenv(v.c_str());
    if(val)
    {
      try
      {
        return boost::lexical_cast<int>(val);
      }
      catch(boost::bad_lexical_cast&)
      {
        return 0;
      }
    }
    else
      return 0;
  }

  void MemoryManagerImpl::checkForOutOfResources()
  {
    if(!isGarbageCollecting && (memCurrent>memHwm || filesCurrent>filesHwm))
    {
      isGarbageCollecting=true;
      thread.schedule(boost::bind(&MemoryManagerImpl::garbageCollect, this));
    }
  }

  void MemoryManagerImpl::garbageCollect()
  {
    printf("+Garbage collector activating. (%llu/%llu files, %llu/%llu MB)\n",
           filesCurrent, filesTotal, memCurrent/1024/1024, memTotal/1024/1024);

    {
      boost::unique_lock<boost::mutex> lock(mut);
      std::map<unsigned long long, std::list<Scroom::Utils::StuffWeak> > sortedInterfaces;

      BOOST_FOREACH(ManagedInfoItem cur, managedInfo)
      {
        sortedInterfaces[cur.second.timestamp].push_back(cur.first);
      }
      std::map<unsigned long long, std::list<Scroom::Utils::StuffWeak> >::iterator cur = sortedInterfaces.begin();
      std::map<unsigned long long, std::list<Scroom::Utils::StuffWeak> >::iterator end = sortedInterfaces.end();
      unloaders = 0;
      unsigned long long filesExpected = filesCurrent;
      unsigned long long memExpected = memCurrent;

      for(;cur!=end && (filesExpected > filesLwm || memExpected > memLwm); ++cur)
      {
        std::list<Scroom::Utils::StuffWeak>& list = cur->second;
        BOOST_FOREACH(Scroom::Utils::StuffWeak r, list)
        {
          ManagedInfo& mi = managedInfo[r];
        
          if(mi.isLoaded)
          {
            MemoryManagedInterface::Ptr o = mi.object.lock();
            if(o)
            {
              boost::unique_lock<boost::mutex> lock(unloadersMut);
              unloaders++;
              CpuBound()->schedule(boost::bind(&MemoryManagerImpl::unload, this, o), PRIO_HIGHEST);
              filesExpected -= mi.fdcount;
              memExpected -= mi.size;
            }
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

    }

    isGarbageCollecting = false;
    
    printf("-Garbage collector terminating. (%llu/%llu files, %llu/%llu MB)\n",
           filesCurrent, filesTotal, memCurrent/1024/1024, memTotal/1024/1024);
  }
  
  void MemoryManagerImpl::unload(MemoryManagedInterface::Ptr i)
  {
    i->do_unload();
    boost::unique_lock<boost::mutex> lock(unloadersMut);
    unloaders--;
    unloadersCond.notify_all();
  }
}
////////////////////////////////////////////////////////////////////////
/// Regular functions

namespace MemoryManager
{
  Scroom::Utils::Stuff registerMMI(MemoryManagedInterface::Ptr object, size_t size, int fdcount)
  {
    return instance()->registerMMI(object, size, fdcount);
  }

  void loadNotification(Scroom::Utils::Stuff r)
  {
    instance()->loadNotification(r);
  }

  void unloadNotification(Scroom::Utils::Stuff r)
  {
    instance()->unloadNotification(r);
  }
}
