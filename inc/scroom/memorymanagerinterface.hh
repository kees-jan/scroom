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
