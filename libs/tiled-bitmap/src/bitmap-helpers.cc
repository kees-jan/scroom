/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

    BitmapSurface::BitmapSurface(int width, int height)
    {
      this->surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
      this->data = NULL;
    }

    BitmapSurface::BitmapSurface(int width, int height, int stride, unsigned char* data)
    {
      this->surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_RGB24, width, height, stride);
      this->data = data;
    }

    
  } // namespace Bitmap
} // namespace Scroom
