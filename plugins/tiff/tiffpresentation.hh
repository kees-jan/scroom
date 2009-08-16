#ifndef _TIFFPRESENTATION_HH
#define _TIFFPRESENTATION_HH

#include <string>

#include <tiledbitmapinterface.hh>
#include <presentationinterface.hh>

typedef struct tiff TIFF;

class TiffPresentation : public PresentationInterface, public SourcePresentation
{
private:
  TIFF* tif;
  int height;
  int width;
  bool negative;
  TiledBitmapInterface* tbi;
  LayerSpec ls;
  
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
