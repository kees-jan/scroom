/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/test/unit_test.hpp>

#include <scroom/rectangle.hh>
#include <scroom/tiledbitmapinterface.hh>

//////////////////////////////////////////////////////////////

class DummyLayerOperations : public LayerOperations
{
public:
  static Ptr create() { return Ptr(new DummyLayerOperations()); }

  int  getBpp() override { return 8; }
  void initializeCairo(cairo_t* /*cr*/) override {}
  void draw(cairo_t* /*cr*/,
            const ConstTile::Ptr& /*tile*/,
            Scroom::Utils::Rectangle<double> /*tileArea*/,
            Scroom::Utils::Rectangle<double> /*viewArea*/,
            int /*zoom*/,
            Scroom::Utils::Stuff /*cache*/) override
  {
  }
  void drawState(cairo_t* /*cr*/, TileState /*s*/, Scroom::Utils::Rectangle<double> /*viewArea*/) override {}
  void reduce(Tile::Ptr /*target*/, const ConstTile::Ptr /*source*/, int /*x*/, int /*y*/) override {}
};

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(TiledBitmap_Tests)

BOOST_AUTO_TEST_CASE(tiledbitmap_can_be_deleted)
{
  LayerSpec ls;
  ls.push_back(DummyLayerOperations::create());
  TiledBitmapInterface::Ptr bitmap = createTiledBitmap(300000, 300000, ls);
  BOOST_CHECK(bitmap);
  std::weak_ptr<TiledBitmapInterface> const weak = bitmap;
  BOOST_CHECK(weak.lock());
  bitmap.reset();
  BOOST_CHECK(!bitmap);
  BOOST_CHECK(!weak.lock());
}

BOOST_AUTO_TEST_SUITE_END()
