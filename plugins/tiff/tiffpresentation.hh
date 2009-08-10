#ifndef _TIFFPRESENTATION_HH
#define _TIFFPRESENTATION_HH

#include <string>

#include <presentationinterface.hh>

class TiffPresentation : public PresentationInterface
{
public:

  virtual ~TiffPresentation();

  bool load(std::string fileName);
  
  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  virtual GdkRectangle getRect();
  virtual void redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom);
};

#endif
