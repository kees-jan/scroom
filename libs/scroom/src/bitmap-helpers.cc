/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/bitmap-helpers.hh>

namespace Scroom
{
  namespace Bitmap
  {

    BitmapSurface::Ptr BitmapSurface::create(int width, int height, cairo_format_t format)
    {
      return BitmapSurface::Ptr(new BitmapSurface(width, height, format));
    }

    BitmapSurface::Ptr BitmapSurface::create(int width, int height, cairo_format_t format,
                                             int stride, boost::shared_ptr<unsigned char> const& data)
    {
      return BitmapSurface::Ptr(new BitmapSurface(width, height, format, stride, data));
    }

    BitmapSurface::~BitmapSurface()
    {
      cairo_surface_destroy(surface);
    }

    cairo_surface_t* BitmapSurface::get()
    {
      return surface;
    }

    BitmapSurface::BitmapSurface(int width, int height, cairo_format_t format)
      : surface(cairo_image_surface_create(format, width, height))
    {}

    BitmapSurface::BitmapSurface(int width, int height, cairo_format_t format,
                                 int stride, boost::shared_ptr<unsigned char> const& data_)
      : surface(cairo_image_surface_create_for_data(data_.get(), format, width, height, stride)), data(data_)
    {}

  } // namespace Bitmap
} // namespace Scroom
