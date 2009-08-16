#ifndef _EXAMPLEPRESENTATION_HH
#define _EXAMPLEPRESENTATION_HH

#include <presentationinterface.hh>

class ExamplePresentation : public PresentationInterface
{
public:
  virtual ~ExamplePresentation();

  virtual GdkRectangle getRect();
  virtual ViewIdentifier* open(ViewInterface* viewInterface);
  virtual void redraw(ViewIdentifier* vid, cairo_t* cr, GdkRectangle presentationArea, int zoom);
  virtual void close(ViewIdentifier* vid);
};

#endif
