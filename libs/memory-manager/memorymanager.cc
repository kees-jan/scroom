#include <memorymanagerinterface.hh>

class MemoryManager
{
private:
  
  
public:
  static MemoryManager* instance();

public:
  MemoryManager();
  ~MemoryManager();

  void registerMMI(MemoryManagedInterface* object, size_t size, int fdcount);
  void unregisterMMI(MemoryManagedInterface* object);
  void loadNotification(MemoryManagedInterface* object);
  void unloadNotification(MemoryManagedInterface* object);
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
}

MemoryManager::~MemoryManager()
{
}

void MemoryManager::registerMMI(MemoryManagedInterface* object, size_t size, int fdcount)
{
}

void MemoryManager::unregisterMMI(MemoryManagedInterface* object)
{
}

void MemoryManager::loadNotification(MemoryManagedInterface* object)
{
}

void MemoryManager::unloadNotification(MemoryManagedInterface* object)
{
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

