#ifndef TEST_HELPERS_HH
#define TEST_HELPERS_HH

#include <boost/shared_ptr.hpp>

#include <gtk/gtk.h>

#include <tiffpresentation.hh>
#include <layeroperations.hh>

#include "measure-framerate-stubs.hh"

extern int drawingAreaWidth;
extern int drawingAreaHeight;

////////////////////////////////////////////////////////////////////////
class TestData
{
public:
  typedef boost::shared_ptr<TestData> Ptr;

private:
  ProgressInterfaceStub* pi;
  ViewInterface* vi;
  TiffPresentation::Ptr tp;
  LayerSpec ls;
  TiledBitmapInterface::Ptr tbi;
  SourcePresentation* sp;
  int zoom;

private:
  TestData(TiffPresentation::Ptr tp, const LayerSpec& ls,
           TiledBitmapInterface::Ptr tbi, SourcePresentation* sp, int zoom);
  
public:
  static Ptr create(TiffPresentation::Ptr tp, const LayerSpec& ls,
                    TiledBitmapInterface::Ptr tbi, SourcePresentation* sp, int zoom);

  ~TestData();

  void redraw(cairo_t* cr);
  bool wait();
};

extern TestData::Ptr testData;

////////////////////////////////////////////////////////////////////////

class Sleep
{
private:
  unsigned int secs;
  bool started;
  struct timespec t;
public:
  Sleep(unsigned int secs);

  bool operator()();
};

////////////////////////////////////////////////////////////////////////

bool quit();
bool reset();
bool wait();

bool setupTest1bpp(int zoom, int width, int height);
bool setupTest2bpp(int zoom, int width, int height);
bool setupTest4bpp(int zoom, int width, int height);
bool setupTest8bpp(int zoom, int width, int height);
bool setupTest8bppColormapped(int zoom, int width, int height);


#endif
