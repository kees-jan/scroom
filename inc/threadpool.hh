#ifndef _THREADPOOL_HH
#define _THREADPOOL_HH

#include <workinterface.hh>

enum
  {
    PRIO_LOWEST,
    PRIO_LOWER,
    PRIO_NORMAL,
    PRIO_HIGHER,
    PRIO_HIGHEST
  };

void schedule(WorkInterface* wi, int priority=PRIO_NORMAL);


#endif
