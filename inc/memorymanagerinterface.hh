#ifndef _MEMORYMANAGERINTERFACE.HH
#define _MEMORYMANAGERINTERFACE.HH

#include <workinterface.hh>

class MemoryManagerInterface
{
  void requestMemoryAvailabilityNotification(size_t amount, WorkInterface* callback);
};


#endif
