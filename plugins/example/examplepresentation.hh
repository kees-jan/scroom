#ifndef _EXAMPLEPRESENTATION_HH
#define _EXAMPLEPRESENTATION_HH

#include <presentationinterface.hh>

class ExamplePresentation : public PresentationInterface
{
public:
  virtual ~ExamplePresentation();

  virtual int getHeight();
  virtual int getWidth();
  virtual void redraw(cairo_t* cr, int left, int top, int right, int bottom, int zoomIn, int zoomOut);
};

#endif
