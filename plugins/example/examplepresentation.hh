#ifndef _EXAMPLEPRESENTATION_HH
#define _EXAMPLEPRESENTATION_HH

#include <presentationinterface.hh>

class ExamplePresentation : public PresentationInterface
{
public:
  virtual ~ExamplePresentation();

  virtual GdkRectangle getRect();
  virtual void redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom);
};

#endif
