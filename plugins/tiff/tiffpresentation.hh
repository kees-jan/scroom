#ifndef _TIFFPRESENTATION_HH
#define _TIFFPRESENTATION_HH

#include <presentationinterface.hh>

class TiffPresentation : public PresentationInterface
{
public:

  virtual ~TiffPresentation();
  
  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  virtual GdkRectangle getRect();
  virtual void redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom);
};

#endif
