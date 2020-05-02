/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

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
      boost::shared_ptr<unsigned char> const data;

    public:
      static Ptr create(int width, int height, cairo_format_t format);
      static Ptr create(int width, int height, cairo_format_t format,
                        int stride, boost::shared_ptr<unsigned char> const& data);

      ~BitmapSurface();

      cairo_surface_t* get();
    private:
      BitmapSurface(int width, int height, cairo_format_t format);
      BitmapSurface(int width, int height, cairo_format_t format,
                    int stride, boost::shared_ptr<unsigned char> const& data);
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
      return static_cast<Base>(((Base(1) << (bpp - 1)) - 1) << 1) | 1;
    }

    template <typename Base>
    PixelIterator<Base>::PixelIterator()
      : currentBase(NULL), currentOffset(0), bpp(0), pixelsPerBase(0), pixelOffset(0), pixelMask(0)
    {
    }

    template <typename Base>
    PixelIterator<Base>::PixelIterator(Base* base, int offset, int bpp_)
      : currentBase(NULL), currentOffset(0), bpp(bpp_), pixelsPerBase(bitsPerBase/bpp), pixelOffset(bpp_), pixelMask(mask(bpp_))
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
        static_cast<Base>((*currentBase & ~(pixelMask << currentOffset*pixelOffset)) | (value << (currentOffset*pixelOffset)));
    }

    template <typename Base>
    inline Base PixelIterator<Base>::operator*()
    {
      return static_cast<Base>((*currentBase>>(currentOffset*pixelOffset)) & pixelMask);
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

