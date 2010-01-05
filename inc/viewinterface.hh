#ifndef _VIEWINTERFACE_HH
#define _VIEWINTERFACE_HH

#include <gtk/gtk.h>

class ViewInterface
{
public:
  
  virtual ~ViewInterface()
  {
  }

  virtual void invalidate()=0;

  virtual GtkProgressBar* getProgressBar()=0;
};


#endif
