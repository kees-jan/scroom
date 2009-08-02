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

  virtual int getHeight()=0;
  virtual int getWidth()=0;
  virtual void redraw(cairo_t* cr, GdkRectangle presentationArea, int zoomIn, int zoomOut)=0;
  
};

#endif
