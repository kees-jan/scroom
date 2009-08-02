#ifndef _PRESENTATIONINTERFACE_H
#define _PRESENTATIONINTERFACE_H

#include <cairo.h>

class PresentationInterface
{
public:
  virtual ~PresentationInterface()
  {
  }

  virtual int getHeight()=0;
  virtual int getWidth()=0;
  virtual void redraw(cairo_t* cr, int left, int top, int right, int bottom, int zoomIn, int zoomOut)=0;
  
};

#endif
