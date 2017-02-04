/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/bitmap-helpers.hh>

namespace Scroom
{
  namespace Bitmap
  {

    BitmapSurface::Ptr BitmapSurface::create(int width, int height)
    {
      return BitmapSurface::Ptr(new BitmapSurface(width, height));
    }

    BitmapSurface::Ptr BitmapSurface::create(int width, int height, int stride, unsigned char* data)
    {
      return BitmapSurface::Ptr(new BitmapSurface(width, height, stride, data));
    }

    BitmapSurface::~BitmapSurface()
    {
      cairo_surface_destroy(surface);
      if(data) free(data);
    }


    cairo_surface_t* BitmapSurface::get()
    {
      return surface;
    }

    BitmapSurface::BitmapSurface(int width, int height) :
        surface(cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height)), data(NULL)
    {}

    BitmapSurface::BitmapSurface(int width, int height, int stride, unsigned char* data) :
        surface(cairo_image_surface_create_for_data(data, CAIRO_FORMAT_RGB24, width, height, stride)), data(data)
    {}

    
  } // namespace Bitmap
} // namespace Scroom
