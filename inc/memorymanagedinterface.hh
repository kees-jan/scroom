#ifndef _MEMORYMANAGED.HH
#define _MEMORYMANAGED.HH

class MemoryManagedInterface
{
public:
  virtual size_t get_size()=0;

  virtual void do_load()=0;
  virtual void do_unload()=0;

  virtual bool request_load()=0;
  virtual void end_request_load()=0;
}

#endif
