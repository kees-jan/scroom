#ifndef _TIFFPRESENTATION_HH
#define _TIFFPRESENTATION_HH

#include <string>
#include <map>

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
  std::map<TiledBitmapViewData*, ViewInterface*> viewData;
  
public:

  TiffPresentation();
  virtual ~TiffPresentation();

  bool load(std::string fileName);
  
  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  virtual GdkRectangle getRect();
  virtual ViewIdentifier* open(ViewInterface* viewInterface);
  virtual void redraw(ViewIdentifier* vid, cairo_t* cr, GdkRectangle presentationArea, int zoom);
  virtual void close(ViewIdentifier* vid);

  ////////////////////////////////////////////////////////////////////////
  // SourcePresentation
  ////////////////////////////////////////////////////////////////////////
private:
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile*>& tiles);

};

#endif
