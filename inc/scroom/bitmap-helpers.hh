/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <type_traits>

#include <boost/operators.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <cairo.h>

#include <scroom/global.hh>

namespace Scroom
{
  namespace Bitmap
  {
    class BitmapSurface
    {
    public:
      typedef boost::shared_ptr<BitmapSurface> Ptr;

    private:
      cairo_surface_t* const                 surface;
      boost::shared_ptr<unsigned char> const data;

    public:
      static Ptr create(int width, int height, cairo_format_t format);
      static Ptr create(int width, int height, cairo_format_t format, int stride, boost::shared_ptr<unsigned char> const& data);

      ~BitmapSurface();
      BitmapSurface(const BitmapSurface&) = delete;
      BitmapSurface(BitmapSurface&&)      = delete;
      BitmapSurface& operator=(const BitmapSurface&) = delete;
      BitmapSurface& operator=(BitmapSurface&&) = delete;

      cairo_surface_t* get();

    private:
      BitmapSurface(int width, int height, cairo_format_t format);
      BitmapSurface(int width, int height, cairo_format_t format, int stride, boost::shared_ptr<unsigned char> const& data);
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
    template <typename ConstBase>
    class SampleIterator : public boost::addable2<SampleIterator<ConstBase>, unsigned int>
    {
    public:
      using Base = typename std::remove_const<ConstBase>::type;

      ConstBase* currentBase;
      int        currentOffset;

      const int        bps;
      const int        samplesPerBase;
      static const int bitsPerBase{8 * sizeof(ConstBase) / sizeof(byte)};
      const int        pixelOffset;
      const ConstBase  pixelMask;

    private:
      static Base mask(int bps) { return (((ConstBase(1) << (bps - 1)) - 1) << 1) | 1; }

    public:
      /**
       * Construct a new SampleIterator
       *
       * @param base Pointer to the element that contains the current sample
       * @param offset number of the sample within that element
       * @param bps Number of bits per sample
       */
      explicit SampleIterator(ConstBase* base, int offset = 0, int bps_ = 1)
        : currentBase(nullptr)
        , currentOffset(0)
        , bps(bps_)
        , samplesPerBase(bitsPerBase / bps)
        , pixelOffset(bps)
        , pixelMask(mask(bps))
      {
        div_t d       = div(offset, samplesPerBase);
        currentBase   = base + d.quot;
        currentOffset = samplesPerBase - 1 - d.rem;
      }

      /** Get the value of the current sample */
      Base get() { return (*currentBase >> (currentOffset * pixelOffset)) & pixelMask; }

      /** Set the value of the current sample */
      void set(ConstBase value)
      {
        *currentBase = (*currentBase & ~(pixelMask << currentOffset * pixelOffset)) | (value << (currentOffset * pixelOffset));
      }

      /** Move to the next sample */
      SampleIterator& operator++()
      {
        // Prefix operator
        if(!(currentOffset--))
        {
          currentOffset = samplesPerBase - 1;
          ++currentBase;
        }

        return *this;
      }

      /** Move to the next sample */
      SampleIterator operator++(int)
      {
        // Postfix operator
        SampleIterator<ConstBase> result = *this;

        if(!(currentOffset--))
        {
          currentOffset = samplesPerBase - 1;
          ++currentBase;
        }

        return result;
      }

      /** Move @c x samples further */
      SampleIterator& operator+=(unsigned int x)
      {
        int   offset = samplesPerBase - 1 - currentOffset + x;
        div_t d      = div(offset, samplesPerBase);
        currentBase += d.quot;
        currentOffset = samplesPerBase - 1 - d.rem;

        return *this;
      }

      /** Get the value of the current sample */
      Base operator*() { return (*currentBase >> (currentOffset * pixelOffset)) & pixelMask; }

      bool operator==(const SampleIterator<ConstBase>& other) const
      {
        return currentBase == other.currentBase && currentOffset == other.currentOffset && bps == other.bps;
      }
    };

  } // namespace Bitmap
} // namespace Scroom
