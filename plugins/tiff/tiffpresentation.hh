#ifndef _TIFFPRESENTATION_HH
#define _TIFFPRESENTATION_HH

#include <string>

#include <scroominterface.hh>
#include <tiledbitmapinterface.hh>
#include <presentationinterface.hh>

typedef struct tiff TIFF;

class TiffPresentation : public PresentationInterface, public SourcePresentation
{
private:
  std::string fileName;
  TIFF* tif;
  int height;
  int width;
  bool negative;
  TiledBitmapInterface* tbi;
  LayerSpec ls;
  int bpp;
  
public:

  TiffPresentation();
  virtual ~TiffPresentation();

  bool load(std::string fileName, FileOperationObserver* observer);
  
  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  virtual GdkRectangle getRect();
  virtual ViewIdentifier* open(ViewInterface* viewInterface);
  virtual void redraw(ViewIdentifier* vid, cairo_t* cr, GdkRectangle presentationArea, int zoom);
  virtual void close(ViewIdentifier* vid);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();

  ////////////////////////////////////////////////////////////////////////
  // SourcePresentation
  ////////////////////////////////////////////////////////////////////////
private:
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);

};

#endif
