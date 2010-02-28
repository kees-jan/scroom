#ifndef _VIEWINTERFACE_HH
#define _VIEWINTERFACE_HH

#include <string>

#include <gtk/gtk.h>

class ViewInterface
{
public:
  
  virtual ~ViewInterface() {}

  virtual void invalidate()=0;

  virtual GtkProgressBar* getProgressBar()=0;

  virtual void addSideWidget(std::string title, GtkWidget* w)=0;
  virtual void removeSideWidget(GtkWidget* w)=0;
};


#endif
