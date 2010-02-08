#ifndef _VIEWINTERFACE_HH
#define _VIEWINTERFACE_HH

#include <string>

#include <gtk/gtk.h>

class ViewInterface
{
public:
  
  virtual ~ViewInterface()
  {
  }

  virtual void invalidate()=0;

  virtual GtkProgressBar* getProgressBar()=0;

  virtual void setTitle(const std::string& title)=0;
};


#endif
