#include <map>

#include <boost/thread.hpp>

#include <memorymanagerinterface.hh>

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
  unsigned int memHwm;
  unsigned int memLwm;
  unsigned int filesHwm;
  unsigned int filesLwm;

  unsigned long memCurrent;
  unsigned long memTotal;
  unsigned long filesCurrent;
  unsigned long filesTotal;

  unsigned long timestamp;

  boost::mutex mut;
  ManagedInfoMap managedInfo;
  
public:
  static MemoryManager* instance();

public:
  MemoryManager();
  ~MemoryManager();

  void registerMMI(MemoryManagedInterface* object, size_t size, int fdcount);
  void unregisterMMI(MemoryManagedInterface* object);
  void loadNotification(MemoryManagedInterface* object);
  void unloadNotification(MemoryManagedInterface* object);

private:
  int fetchFromEnvironment(std::string v);
};

////////////////////////////////////////////////////////////////////////
/// MemoryManager

MemoryManager* MemoryManager::instance()
{
  static MemoryManager m;
  return &m;
}

MemoryManager::MemoryManager()
{
  printf("Creating memory manager\n");

  memHwm = fetchFromEnvironment(SCROOM_MEMORY_HWM);
  memLwm = fetchFromEnvironment(SCROOM_MEMORY_LWM);
  filesHwm = fetchFromEnvironment(SCROOM_FILES_HWM);
  filesLwm = fetchFromEnvironment(SCROOM_FILES_LWM);

  if(!memHwm)
  {
    memHwm = 2048;
    memLwm = 1920;
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

  printf("Memory watermarks: High %dMB, Low %dMB\n", memHwm, memLwm);
  printf("File watermarks: High %d, Low %d\n", filesHwm, filesLwm);
  
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
      memCurrent-=m.size;
      filesCurrent-=m.fdcount;
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
    memCurrent += m.size;
    filesCurrent += m.fdcount;
  }
}

void MemoryManager::unloadNotification(MemoryManagedInterface* object)
{
  ManagedInfo& m = managedInfo[object];
  assert(m.size!=0 || m.fdcount!=0);

  if(m.isLoaded)
  {
    m.isLoaded = false;
    memCurrent -= m.size;
    filesCurrent -= m.fdcount;
  }
}

int MemoryManager::fetchFromEnvironment(std::string v)
{
  std::string value = getenv(v.c_str());
  return atoi(value.c_str());
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

