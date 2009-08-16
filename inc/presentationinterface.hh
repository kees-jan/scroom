#ifndef _PRESENTATIONINTERFACE_H
#define _PRESENTATIONINTERFACE_H

#include <gdk/gdk.h>
#include <cairo.h>

#include <viewinterface.hh>

class ViewIdentifier
{
public:
  virtual ~ViewIdentifier()
  {}
};

class PresentationInterface
{
public:
  virtual ~PresentationInterface()
  {
  }

  virtual GdkRectangle getRect()=0;

  virtual ViewIdentifier* open(ViewInterface* viewInterface)=0;
  virtual void redraw(ViewIdentifier* vid, cairo_t* cr, GdkRectangle presentationArea, int zoom)=0;
  virtual void close(ViewIdentifier* vid)=0;
  
};

#endif
