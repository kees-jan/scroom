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

#ifndef _BITMAP_HELPERS_HH
#define _BITMAP_HELPERS_HH

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <scroom/global.hh>

#include <cairo.h>

namespace Scroom
{
  namespace Bitmap
  {
    class BitmapSurface : public boost::noncopyable
    {
    public:
      typedef boost::shared_ptr<BitmapSurface> Ptr;

    private:
      cairo_surface_t* const surface;
      unsigned char* const data;
  
    public:
      static Ptr create(int width, int height);
      static Ptr create(int width, int height, int stride, unsigned char* data);

      ~BitmapSurface();

      cairo_surface_t* get();
    private:
      BitmapSurface(int width, int heght);
      BitmapSurface(int width, int height, int stride, unsigned char* data);
    };

    ////////////////////////////////////////////////////////////////////////
    // PixelIterator

    template <typename Base>
    class PixelIterator
    {
    private:
      Base* currentBase;
      int currentOffset;
      const int bpp;
      const int pixelsPerBase;
      static const int bitsPerBase;
      const int pixelOffset;
      const Base pixelMask;

    private:
      static Base mask(int bpp);
  
    public:
      PixelIterator();
      PixelIterator(Base* base, int offset=0, int bpp=1);
      Base get();
      void set(Base value);
      PixelIterator& operator++();
      PixelIterator operator++(int);
      PixelIterator& operator+=(int x);
      Base operator*();
    };

    template <typename Base>
    const int PixelIterator<Base>::bitsPerBase = 8*sizeof(Base)/sizeof(byte);

    template <typename Base>
    Base PixelIterator<Base>::mask(int bpp)
    {
      return (((Base(1) << (bpp - 1)) - 1) << 1) | 1;
    }

    template <typename Base>
    PixelIterator<Base>::PixelIterator()
      : currentBase(NULL), currentOffset(0), bpp(0), pixelsPerBase(0), pixelOffset(0), pixelMask(0)
    {
    }

    template <typename Base>
    PixelIterator<Base>::PixelIterator(Base* base, int offset, int bpp)
      : currentBase(NULL), currentOffset(0), bpp(bpp), pixelsPerBase(bitsPerBase/bpp), pixelOffset(bpp), pixelMask(mask(bpp))
    {
      div_t d = div(offset, pixelsPerBase);
      currentBase = base+d.quot;
      currentOffset = pixelsPerBase-1-d.rem;
    }

    template <typename Base>
    inline Base PixelIterator<Base>::get()
    {
      return (*currentBase>>(currentOffset*pixelOffset)) & pixelMask;
    }

    template <typename Base>
    inline void PixelIterator<Base>::set(Base value)
    {
      *currentBase =
        (*currentBase & ~(pixelMask << (currentOffset*pixelOffset))) |
        (value  << (currentOffset*pixelOffset));
    }

    template <typename Base>
    inline Base PixelIterator<Base>::operator*()
    {
      return (*currentBase>>(currentOffset*pixelOffset)) & pixelMask;
    }

    template <typename Base>
    inline PixelIterator<Base>& PixelIterator<Base>::operator++()
    {
      // Prefix operator
      if(!(currentOffset--))
      {
        currentOffset=pixelsPerBase-1;
        ++currentBase;
      }
  
      return *this;
    }

    template <typename Base>
    inline PixelIterator<Base> PixelIterator<Base>::operator++(int)
    {
      // Postfix operator
      PixelIterator<Base> result = *this;
  
      if(!(currentOffset--))
      {
        currentOffset=pixelsPerBase-1;
        ++currentBase;
      }
  
      return result;
    }

    template <typename Base>
    PixelIterator<Base>& PixelIterator<Base>::operator+=(int x)
    {
      int offset = pixelsPerBase-1-currentOffset+x;
      div_t d = div(offset, pixelsPerBase);
      currentBase += d.quot;
      currentOffset = pixelsPerBase-1-d.rem;

      return *this;
    }
  }
}
#endif
