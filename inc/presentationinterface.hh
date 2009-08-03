#ifndef _PRESENTATIONINTERFACE_H
#define _PRESENTATIONINTERFACE_H

#include <gdk/gdk.h>
#include <cairo.h>

class PresentationInterface
{
public:
  virtual ~PresentationInterface()
  {
  }

  virtual GdkRectangle getRect()=0;
  virtual void redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom)=0;
  
};

#endif
