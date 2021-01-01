/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/test/unit_test.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/rectangle.hh>
#include <scroom/tiledbitmapinterface.hh>

//////////////////////////////////////////////////////////////

class DummyLayerOperations : public LayerOperations
{
public:
  static Ptr create() { return Ptr(new DummyLayerOperations()); }

  int  getBpp() override { return 8; }
  void initializeCairo(cairo_t*) override {}
  void draw(cairo_t*,
            const ConstTile::Ptr,
            Scroom::Utils::Rectangle<double>,
            Scroom::Utils::Rectangle<double>,
            int,
            Scroom::Utils::Stuff) override
  {}
  void drawState(cairo_t*, TileState, Scroom::Utils::Rectangle<double>) override {}
  void reduce(Tile::Ptr, const ConstTile::Ptr, int, int) override {}
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
