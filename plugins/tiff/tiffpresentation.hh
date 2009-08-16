#ifndef _TIFFPRESENTATION_HH
#define _TIFFPRESENTATION_HH

#include <string>

#include <presentationinterface.hh>

typedef struct tiff TIFF;

class TiffPresentation : public PresentationInterface
{
private:
  TIFF* tif;
  int height;
  int width;
  bool negative;
  
public:

  TiffPresentation();
  virtual ~TiffPresentation();

  bool load(std::string fileName);
  
  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  virtual GdkRectangle getRect();
  virtual void redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom);
};

#endif
