/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <scroom/tiledbitmapinterface.hh>

#include <boost/test/unit_test.hpp>
#include <boost/weak_ptr.hpp>


//////////////////////////////////////////////////////////////

class DummyLayerOperations: public LayerOperations
{
public:
  static Ptr create() { return Ptr(new DummyLayerOperations()); }
  virtual ~DummyLayerOperations() {}

  virtual int getBpp() { return 8; }
  virtual void initializeCairo(cairo_t*) {}
  virtual void draw(cairo_t*, const ConstTile::Ptr, GdkRectangle, GdkRectangle, int, Scroom::Utils::Stuff) {}
  virtual void drawState(cairo_t*, TileState, GdkRectangle) {}
  virtual void reduce(Tile::Ptr, const ConstTile::Ptr, int, int) {}
};

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(TiledBitmap_Tests)

BOOST_AUTO_TEST_CASE(tiledbitmap_can_be_deleted)
{
  LayerSpec ls;
  ls.push_back(DummyLayerOperations::create());
  TiledBitmapInterface::Ptr bitmap = createTiledBitmap(300000, 300000, ls);
  BOOST_CHECK(bitmap);
  boost::weak_ptr<TiledBitmapInterface> weak = bitmap;
  BOOST_CHECK(weak.lock());
  bitmap.reset();
  BOOST_CHECK(!bitmap);
  BOOST_CHECK(!weak.lock());
}

BOOST_AUTO_TEST_SUITE_END()
