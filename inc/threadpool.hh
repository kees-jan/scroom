#ifndef _THREADPOOL_HH
#define _THREADPOOL_HH

#include <workinterface.hh>

enum
  {
    PRIO_HIGHEST = 100,
    PRIO_HIGHER,
    PRIO_HIGH,
    PRIO_NORMAL,
    PRIO_LOW,
    PRIO_LOWER,
    PRIO_LOWEST,
  };

void schedule(WorkInterface* wi, int priority=PRIO_NORMAL);


#endif
