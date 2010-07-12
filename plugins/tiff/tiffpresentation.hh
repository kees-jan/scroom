#ifndef _TIFFPRESENTATION_HH
#define _TIFFPRESENTATION_HH

#include <string>
#include <map>
#include <list>

#include <scroominterface.hh>
#include <tiledbitmapinterface.hh>
#include <presentationinterface.hh>
#include <colormappable.hh>

typedef struct tiff TIFF;

class TiffPresentation : public PresentationInterface, public SourcePresentation, public Colormappable
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
  std::map<std::string, std::string> properties;
  std::list<ViewInterface*> views;
  Colormap::Ptr colormap;
  
public:

  TiffPresentation();
  virtual ~TiffPresentation();

  bool load(std::string fileName, FileOperationObserver* observer);
  
  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  virtual GdkRectangle getRect();
  virtual void open(ViewInterface* viewInterface);
  virtual void redraw(ViewInterface* vi, cairo_t* cr, GdkRectangle presentationArea, int zoom);
  virtual void close(ViewInterface* vi);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();

  ////////////////////////////////////////////////////////////////////////
  // SourcePresentation
  ////////////////////////////////////////////////////////////////////////
private:
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles);
 
  ////////////////////////////////////////////////////////////////////////
  // Colormappable
  ////////////////////////////////////////////////////////////////////////

public:
  void registerObserver(Viewable* observer);
  void setColormap(Colormap::Ptr colormap);
  Colormap::Ptr getColormap();
  int getNumberOfColors();

  ////////////////////////////////////////////////////////////////////////
  // Helpers
  ////////////////////////////////////////////////////////////////////////
public:
  
};

#endif
