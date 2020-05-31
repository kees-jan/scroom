/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/operators.hpp>

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
    // SampleIterator

    /**
     * Iterate over samples (bits) within a larger type, for example a byte
     * 
     * Allows you to extract (or set) specific bits in a larger datatype, 
     * as well as iterate over samples within the larger datatype, or even
     * continuing with the next, if you reach the end of the current one
     * 
     * @param Base the larger type that contains the samples. Typically uint8_t
     */
    template <typename Base>
    class SampleIterator : public boost::addable2<SampleIterator<Base>, unsigned int>
    {
    public:
      Base* currentBase;
      int currentOffset;

      const int bps;
      const int samplesPerBase;
      static const int bitsPerBase;
      const int pixelOffset;
      const Base pixelMask;

    private:
      static Base mask(int bps);

    public:
      /**
       * Construct a new SampleIterator
       * 
       * @param base Pointer to the element that contains the current sample
       * @param offset number of the sample within that element
       * @param bps Number of bits per sample
       */
      SampleIterator(Base* base, int offset=0, int bps=1);

      /** Get the value of the current sample */
      Base get();

      /** Set the value of the current sample */
      void set(Base value);

      /** Move to the next sample */
      SampleIterator& operator++();

      /** Move to the next sample */
      SampleIterator operator++(int);

      /** Move @c x samples further */
      SampleIterator& operator+=(unsigned int x);

      /** Get the value of the current sample */
      Base operator*();

      bool operator==(const SampleIterator<Base>& other) const;

    };

    template <typename Base>
    const int SampleIterator<Base>::bitsPerBase = 8*sizeof(Base)/sizeof(byte);

    template <typename Base>
    Base SampleIterator<Base>::mask(int bps)
    {
      return (((Base(1) << (bps - 1)) - 1) << 1) | 1;
    }

    template <typename Base>
    SampleIterator<Base>::SampleIterator(Base* base, int offset, int bps_)
      : currentBase(NULL), currentOffset(0), bps(bps_), samplesPerBase(bitsPerBase/bps_), pixelOffset(bps_), pixelMask(mask(bps_))
    {
      div_t d = div(offset, samplesPerBase);
      currentBase = base+d.quot;
      currentOffset = samplesPerBase-1-d.rem;
    }

    template <typename Base>
    inline Base SampleIterator<Base>::get()
    {
      return (*currentBase>>(currentOffset*pixelOffset)) & pixelMask;
    }

    template <typename Base>
    inline void SampleIterator<Base>::set(Base value)
    {
      *currentBase = (*currentBase & ~(pixelMask << currentOffset*pixelOffset)) | (value << (currentOffset*pixelOffset));
    }

    template <typename Base>
    inline Base SampleIterator<Base>::operator*()
    {
      return (*currentBase>>(currentOffset*pixelOffset)) & pixelMask;
    }

    template <typename Base>
    inline SampleIterator<Base>& SampleIterator<Base>::operator++()
    {
      // Prefix operator
      if(!(currentOffset--))
      {
        currentOffset=samplesPerBase-1;
        ++currentBase;
      }

      return *this;
    }

    template <typename Base>
    inline SampleIterator<Base> SampleIterator<Base>::operator++(int)
    {
      // Postfix operator
      SampleIterator<Base> result = *this;

      if(!(currentOffset--))
      {
        currentOffset=samplesPerBase-1;
        ++currentBase;
      }

      return result;
    }

    template <typename Base>
    SampleIterator<Base>& SampleIterator<Base>::operator+=(unsigned int x)
    {
      int offset = samplesPerBase-1-currentOffset+x;
      div_t d = div(offset, samplesPerBase);
      currentBase += d.quot;
      currentOffset = samplesPerBase-1-d.rem;

      return *this;
    }

    template <typename Base>
    bool SampleIterator<Base>::operator==(const SampleIterator<Base>& other) const
    {
      return currentBase == other.currentBase && currentOffset == other.currentOffset && bps == other.bps;
    }

  }
}

