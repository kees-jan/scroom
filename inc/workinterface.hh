#ifndef _WORKINTERFACE_H
#define _WORKINTERFACE_H

class WorkInterface
{
public:
  
  virtual ~WorkInterface()
  {
  }

  virtual bool doWork()=0;
};


#endif
