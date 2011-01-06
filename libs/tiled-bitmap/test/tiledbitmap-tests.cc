#include <scroom/tiledbitmapinterface.hh>

#include <boost/test/unit_test.hpp>
#include <boost/weak_ptr.hpp>


//////////////////////////////////////////////////////////////

class DummyLayerOperations: public LayerOperations
{
  virtual ~DummyLayerOperations() {}

  virtual int getBpp() { return 8; }
  virtual void initializeCairo(cairo_t*) {}
  virtual void draw(cairo_t*, const Tile::Ptr, GdkRectangle, GdkRectangle, int) {}
  virtual void drawState(cairo_t*, TileState, GdkRectangle) {}
  virtual void reduce(Tile::Ptr, const Tile::Ptr, int, int) {}
};

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(TiledBitmap_Tests)

BOOST_AUTO_TEST_CASE(tiledbitmap_can_be_deleted)
{
  LayerSpec ls;
  ls.push_back(new DummyLayerOperations());
  TiledBitmapInterface::Ptr bitmap = createTiledBitmap(300000, 300000, ls, FileOperationObserver::Ptr());
  BOOST_CHECK(bitmap);
  boost::weak_ptr<TiledBitmapInterface> weak = bitmap;
  BOOST_CHECK(weak.lock());
  bitmap.reset();
  BOOST_CHECK(!bitmap);
  BOOST_CHECK(!weak.lock());
}

BOOST_AUTO_TEST_SUITE_END()
