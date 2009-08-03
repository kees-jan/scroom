#ifndef _VIEWINTERFACE_HH
#define _VIEWINTERFACE_HH

class ViewInterface
{
public:
  
  virtual ~ViewInterface()
  {
  }

  virtual void invalidate()=0;
};


#endif
