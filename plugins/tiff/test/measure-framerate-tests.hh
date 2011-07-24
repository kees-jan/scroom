/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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
#ifndef MEASURE_FRAMERATE_TESTS_HH
#define MEASURE_FRAMERATE_TESTS_HH

#include <boost/shared_ptr.hpp>

#include <gtk/gtk.h>

#include <tiffpresentation.hh>
#include <layeroperations.hh>

#include "measure-framerate-stubs.hh"

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

void init_tests();


#endif
