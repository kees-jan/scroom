#ifndef _THREADPOOL_HH
#define _THREADPOOL_HH

#include <workinterface.hh>

enum
  {
    PRIO_LOWEST=100,
    PRIO_LOWER,
    PRIO_LOW,
    PRIO_NORMAL,
    PRIO_HIGH,
    PRIO_HIGHER,
    PRIO_HIGHEST
  };

void schedule(WorkInterface* wi, int priority=PRIO_NORMAL);


#endif
