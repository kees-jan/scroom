/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <bit>
#include <cstdint>
#include <cstring>

#include <scroom/assertions.hh>
#include <scroom/bitmap-helpers.hh>
#include <scroom/layer-reduction.hh>

using namespace Scroom::Bitmap;

namespace Scroom::TiledBitmap
{
  namespace
  {

    void reduce1bpp(const Tile::Ptr& target, const ConstTile::Ptr& source, int x, int y)
    {
      // Reducing by a factor 8. Source tile is 1bpp. Target tile is 8bpp
      const int sourceStride = source->width / 8;
      const byte* sourceBase = source->data.get();

      const int targetStride = target->width;
      byte* targetBase = target->data.get() + target->height * y * targetStride / 8 + target->width * x / 8;

      for(int j = 0; j < source->height / 8; j++, targetBase += targetStride, sourceBase += sourceStride * 8)
      {
        // Iterate vertically over target
        const byte* sourcePtr = sourceBase;
        byte* targetPtr = targetBase;

        for(int i = 0; i < source->width / 8; i++, sourcePtr++, targetPtr++)
        {
          // Iterate horizontally over target

          // Map the fraction of set source bits to [0, 255].

          const byte* current = sourcePtr;
          int sum = 0;
          for(int k = 0; k < 8; k++, current += sourceStride)
          {
            sum += std::popcount(*current);
          }

          *targetPtr = sum * 255 / 64;
        }
      }
    }

    void reduce8bpp(const Tile::Ptr& target, const ConstTile::Ptr& source, int x, int y)
    {
      // Reducing by a factor 8. Source tile is 8bpp. Target tile is 8bpp
      const int sourceStride = source->width;
      const byte* sourceBase = source->data.get();

      const int targetStride = target->width;
      byte* targetBase = target->data.get() + target->height * y * targetStride / 8 + target->width * x / 8;

      for(int j = 0; j < source->height / 8; j++, targetBase += targetStride, sourceBase += sourceStride * 8)
      {
        // Iterate vertically over target
        const byte* sourcePtr = sourceBase;
        byte* targetPtr = targetBase;

        for(int i = 0; i < source->width / 8; i++, sourcePtr += 8, targetPtr++)
        {
          // Iterate horizontally over target

          // Goal is to compute a 8-bit grey value from a 8*8 grey image.
          const byte* base = sourcePtr;
          int sum = 0;
          for(int k = 0; k < 8; k++, base += sourceStride)
          {
            const byte* current = base;
            for(int l = 0; l < 8; l++, current++)
            {
              sum += *current;
            }
          }

          *targetPtr = sum / 64;
        }
      }
    }

    void reduce16bpp(const Tile::Ptr& target, const ConstTile::Ptr& source, int x, int y)
    {
      // Reducing by a factor 8. Source tile is 16bpp. Target tile is 16bpp
      const int sourceStride = source->width;
      auto const* sourceBase = reinterpret_cast<uint16_t const*>(source->data.get());

      const int targetStride = target->width;
      auto* targetBase =
        reinterpret_cast<uint16_t*>(target->data.get()) + target->height * y * targetStride / 8 + targetStride * x / 8;

      for(int j = 0; j < source->height / 8; j++, targetBase += targetStride, sourceBase += sourceStride * 8)
      {
        // Iterate vertically over target
        auto const* sourcePtr = sourceBase;
        auto* targetPtr = targetBase;

        for(int i = 0; i < source->width / 8; i++, sourcePtr += 8, targetPtr++)
        {
          // Iterate horizontally over target

          // Goal is to compute a 16-bit grey value from a 8*8 grey image.
          auto const* base = sourcePtr;
          int sum = 0;
          for(int k = 0; k < 8; k++, base += sourceStride)
          {
            auto const* current = base;
            for(int l = 0; l < 8; l++, current++)
            {
              sum += *current;
            }
          }

          *targetPtr = sum / 64;
        }
      }
    }

    void reduceRgb24bpp(const Tile::Ptr& target, const ConstTile::Ptr& source, int x, int y)
    {
      // Reducing by a factor 8. Source tile is 24bpp. Target tile is 24bpp
      const int sourceStride = 3 * source->width; // stride in bytes
      const byte* sourceBase = source->data.get();

      const int targetStride = 3 * target->width; // stride in bytes
      byte* targetBase = target->data.get() + target->height * y * targetStride / 8 + targetStride * x / 8;

      for(int j = 0; j < source->height / 8; j++, targetBase += targetStride, sourceBase += sourceStride * 8)
      {
        // Iterate vertically over target
        const byte* sourcePtr = sourceBase;
        byte* targetPtr = targetBase;

        for(int i = 0; i < source->width / 8; i++, sourcePtr += 8 * 3, targetPtr += 3)
        {
          // Iterate horizontally over target

          // Goal is to compute a average RGB value from a 8*8 RGB image.
          const byte* base = sourcePtr;
          int sum_r = 0;
          int sum_g = 0;
          int sum_b = 0;
          for(int k = 0; k < 8; k++, base += sourceStride)
          {
            const byte* current = base;
            for(int l = 0; l < 8; l++, current += 3)
            {
              sum_r += current[0];
              sum_g += current[1];
              sum_b += current[2];
            }
          }
          targetPtr[0] = sum_r / 64;
          targetPtr[1] = sum_g / 64;
          targetPtr[2] = sum_b / 64;
        }
      }
    }

    void reduceRgb48bpp(const Tile::Ptr& target, const ConstTile::Ptr& source, int x, int y)
    {
      // Reducing by a factor 8. Source tile is 48bpp. Target tile is 48bpp
      const int sourceStride = 3 * source->width; // stride in uint16_t samples
      auto const* sourceBase = reinterpret_cast<uint16_t const*>(source->data.get());

      const int targetStride = 3 * target->width; // stride in uint16_t samples
      auto* targetBase =
        reinterpret_cast<uint16_t*>(target->data.get()) + target->height * y * targetStride / 8 + targetStride * x / 8;

      for(int j = 0; j < source->height / 8; j++, targetBase += targetStride, sourceBase += sourceStride * 8)
      {
        // Iterate vertically over target
        auto const* sourcePtr = sourceBase;
        auto* targetPtr = targetBase;

        for(int i = 0; i < source->width / 8; i++, sourcePtr += 8 * 3, targetPtr += 3)
        {
          // Iterate horizontally over target

          // Goal is to compute an average RGB value from an 8*8 RGB image.
          auto const* base = sourcePtr;
          int sum_r = 0;
          int sum_g = 0;
          int sum_b = 0;
          for(int k = 0; k < 8; k++, base += sourceStride)
          {
            auto const* current = base;
            for(int l = 0; l < 8; l++, current += 3)
            {
              sum_r += current[0];
              sum_g += current[1];
              sum_b += current[2];
            }
          }
          targetPtr[0] = sum_r / 64;
          targetPtr[1] = sum_g / 64;
          targetPtr[2] = sum_b / 64;
        }
      }
    }

    void reduceBinaryWithOr(const Tile::Ptr& target, const ConstTile::Ptr& source, int x, int y)
    {
      // Reducing by a factor 8. Source tile is 1bpp. Target tile is 1bpp
      const int sourceStride = source->width / 8;
      const byte* sourceBase = source->data.get();

      const int targetStride = target->width / 8;
      byte* targetBase = target->data.get() + target->height * y * targetStride / 8 + target->width * x / 8 / 8;

      for(int j = 0; j < source->height / 8; j++, targetBase += targetStride, sourceBase += sourceStride * 8)
      {
        // Iterate vertically over target
        const byte* sourcePtr = sourceBase;
        SampleIterator<byte> targetPtr(targetBase, 0);

        for(int i = 0; i < source->width / 8; i++, sourcePtr++, targetPtr++)
        {
          // Iterate horizontally over target

          // Any set source bit makes the reduced bit set.

          const byte* current = sourcePtr;
          int sum = 0;
          for(int k = 0; k < 8; k++, current += sourceStride)
          {
            sum += std::popcount(*current);
          }

          targetPtr.set((sum > 0) ? 1 : 0);
        }
      }
    }

    void reduceCmyk4(const Tile::Ptr& target, const ConstTile::Ptr& source, int top_left_x, int top_left_y)
    {
      // Reducing by a factor 8
      const int sourceStride = source->width / 2; // stride in bytes
      const byte* sourceBase = source->data.get();

      const int targetStride = 8 * target->width / 2; // stride in bytes
      byte* targetBase = target->data.get() + (target->height * top_left_y + top_left_x) * targetStride / 8;

      for(int y = 0; y < source->height / 8; y++)
      {
        for(int x = 0; x < source->width / 8; x++)
        {
          const byte* base = sourceBase + 4 * x; // start of the row
          const byte* end = base + 8 * sourceStride; // end of the row

          int sum_c = 0;
          int sum_m = 0;
          int sum_y = 0;
          int sum_k = 0;
          for(const byte* row = base; row < end; row += sourceStride)
          {
            for(size_t current = 0; current < 4; current++)
            {
              sum_c += ((row[current] >> 7) & 1) + ((row[current] >> 3) & 1);
              sum_m += ((row[current] >> 6) & 1) + ((row[current] >> 2) & 1);
              sum_y += ((row[current] >> 5) & 1) + ((row[current] >> 1) & 1);
              sum_k += ((row[current] >> 4) & 1) + ((row[current] >> 0) & 1);
            }
          }

          targetBase[4 * x] = static_cast<uint8_t>(sum_c * 255 / 64);
          targetBase[4 * x + 1] = static_cast<uint8_t>(sum_m * 255 / 64);
          targetBase[4 * x + 2] = static_cast<uint8_t>(sum_y * 255 / 64);
          targetBase[4 * x + 3] = static_cast<uint8_t>(sum_k * 255 / 64);
        }

        targetBase += targetStride;
        sourceBase += sourceStride * 8;
      }
    }

    void reduceCmyk8(const Tile::Ptr& target, const ConstTile::Ptr& source, int top_left_x, int top_left_y)
    {
      // Reducing by a factor 8
      const int sourceStride = source->width; // stride in bytes
      const byte* sourceBase = source->data.get();

      const int targetStride = 8 * target->width / 2; // stride in bytes
      byte* targetBase = target->data.get() + (target->height * top_left_y + top_left_x) * targetStride / 8;

      for(int y = 0; y < source->height / 8; y++)
      {
        for(int x = 0; x < source->width / 8; x++)
        {
          const byte* base = sourceBase + 2 * 4 * x; // start of the row
          const byte* end = base + 8 * sourceStride; // end of the row

          int sum_c = 0;
          int sum_m = 0;
          int sum_y = 0;
          int sum_k = 0;
          for(const byte* row = base; row < end; row += sourceStride)
          {
            for(size_t current = 0; current < 8; current++)
            {
              sum_c += row[current] >> 6;
              sum_m += (row[current] & 0x30) >> 4;
              sum_y += (row[current] & 0x0C) >> 2;
              sum_k += row[current] & 0x03;
            }
          }

          targetBase[4 * x + 0] = static_cast<byte>(sum_c * 255 / 192);
          targetBase[4 * x + 1] = static_cast<byte>(sum_m * 255 / 192);
          targetBase[4 * x + 2] = static_cast<byte>(sum_y * 255 / 192);
          targetBase[4 * x + 3] = static_cast<byte>(sum_k * 255 / 192);
        }

        targetBase += targetStride;
        sourceBase += sourceStride * 8;
      }
    }

    void reduceCmyk16(const Tile::Ptr& target, const ConstTile::Ptr& source, int top_left_x, int top_left_y)
    {
      // Reducing by a factor 8
      const int sourceStride = 4 * source->width / 2; // stride in bytes
      const byte* sourceBase = source->data.get();

      const int targetStride = 8 * target->width / 2; // stride in bytes
      byte* targetBase = target->data.get() + (target->height * top_left_y + top_left_x) * targetStride / 8;

      for(int y = 0; y < source->height / 8; y++)
      {
        byte* targetPtr = targetBase;

        for(int x = 0; x < source->width / 8; x++)
        {
          const byte* base = sourceBase + 4 * 4 * x; // start of the row
          const byte* end = base + 8 * sourceStride; // end of the row

          int sum_c = 0;
          int sum_m = 0;
          int sum_y = 0;
          int sum_k = 0;
          for(const byte* row = base; row < end; row += sourceStride)
          {
            for(size_t current = 0; current < 8 * 2; current += 2)
            {
              sum_c += row[current] >> 4;
              sum_m += row[current] & 15;
              sum_y += row[current + 1] >> 4;
              sum_k += row[current + 1] & 15;
            }
          }

          targetPtr[0] = static_cast<byte>(sum_c * 255 / (15 * 64));
          targetPtr[1] = static_cast<byte>(sum_m * 255 / (15 * 64));
          targetPtr[2] = static_cast<byte>(sum_y * 255 / (15 * 64));
          targetPtr[3] = static_cast<byte>(sum_k * 255 / (15 * 64));

          targetPtr += 4;
        }

        targetBase += targetStride;
        sourceBase += sourceStride * 8;
      }
    }

    void reduceCmyk32(const Tile::Ptr& target, const ConstTile::Ptr& source, int top_left_x, int top_left_y)
    {
      // Reducing by a factor 8
      const int sourceStride = 8 * source->width / 2; // stride in bytes
      const byte* sourceBase = source->data.get();

      const int targetStride = 8 * target->width / 2; // stride in bytes
      byte* targetBase = target->data.get() + (target->height * top_left_y + top_left_x) * targetStride / 8;

      for(int y = 0; y < source->height / 8; y++)
      {
        byte* targetPtr = targetBase;

        for(int x = 0; x < source->width / 8; x++)
        {
          const byte* base = sourceBase + 8 * 4 * x; // start of the row
          const byte* end = base + 8 * sourceStride; // end of the row

          int sum_c = 0;
          int sum_m = 0;
          int sum_y = 0;
          int sum_k = 0;
          for(const byte* row = base; row < end; row += sourceStride)
          {
            for(size_t current = 0; current < 8 * 4; current += 4)
            {
              sum_c += row[current];
              sum_m += row[current + 1];
              sum_y += row[current + 2];
              sum_k += row[current + 3];
            }
          }

          targetPtr[0] = static_cast<byte>(sum_c / 64);
          targetPtr[1] = static_cast<byte>(sum_m / 64);
          targetPtr[2] = static_cast<byte>(sum_y / 64);
          targetPtr[3] = static_cast<byte>(sum_k / 64);

          targetPtr += 4;
        }

        targetBase += targetStride;
        sourceBase += sourceStride * 8;
      }
    }

    void reduceCmyk64(const Tile::Ptr& target, const ConstTile::Ptr& source, int top_left_x, int top_left_y)
    {
      // Reducing by a factor 8. Source tile is 64bpp. Target tile is 64bpp
      const int sourceStride = 4 * source->width; // stride in uint16_t samples
      auto const* sourceBase = reinterpret_cast<uint16_t const*>(source->data.get());

      const int targetStride = 4 * target->width; // stride in uint16_t samples
      uint16_t* targetBase =
        reinterpret_cast<uint16_t*>(target->data.get()) + (target->height * top_left_y + top_left_x) * targetStride / 8;

      for(int y = 0; y < source->height / 8; y++)
      {
        uint16_t* targetPtr = targetBase;

        for(int x = 0; x < source->width / 8; x++)
        {
          const uint16_t* base = sourceBase + 8 * 4 * x;
          const uint16_t* end = base + 8 * sourceStride;

          int sum_c = 0;
          int sum_m = 0;
          int sum_y = 0;
          int sum_k = 0;
          for(const uint16_t* row = base; row < end; row += sourceStride)
          {
            for(size_t current = 0; current < 8 * 4; current += 4)
            {
              sum_c += row[current];
              sum_m += row[current + 1];
              sum_y += row[current + 2];
              sum_k += row[current + 3];
            }
          }

          targetPtr[0] = static_cast<uint16_t>(sum_c / 64);
          targetPtr[1] = static_cast<uint16_t>(sum_m / 64);
          targetPtr[2] = static_cast<uint16_t>(sum_y / 64);
          targetPtr[3] = static_cast<uint16_t>(sum_k / 64);

          targetPtr += 4;
        }

        targetBase += targetStride;
        sourceBase += sourceStride * 8;
      }
    }


    void reduceColormap(int bitsPerIndex, const Tile::Ptr& target, const ConstTile::Ptr& source, int x, int y)
    {
      const unsigned pixelsPerByte = 8 / bitsPerIndex;
      const unsigned pixelMask = (1U << bitsPerIndex) - 1;

      // The reduced format stores the two most frequent colormap indices.
      const int sourceStride = source->width / pixelsPerByte;
      const byte* sourceBase = source->data.get();

      const int targetMultiplier = 2; // two indices per pixel
      const int targetStride = targetMultiplier * target->width / pixelsPerByte;
      byte* targetBase =
        target->data.get() + target->height * targetStride * y / 8 + targetMultiplier * target->width * x / 8 / pixelsPerByte;

      for(int j = 0; j < source->height / 8; j++, targetBase += targetStride, sourceBase += sourceStride * 8)
      {
        // Iterate vertically over target
        const byte* sourcePtr = sourceBase;
        SampleIterator<uint16_t> targetPtr(reinterpret_cast<uint16_t*>(targetBase), 0, targetMultiplier * bitsPerIndex);

        for(int i = 0; i < source->width / 8; i++, sourcePtr += 8 / pixelsPerByte, ++targetPtr)
        {
          // Iterate horizontally over target

          // Goal is to determine which values occurs most often in a 8*8
          // rectangle, and pick the top two.
          const byte* base = sourcePtr;
          byte lookup[pixelMask + 1];
          memset(lookup, 0, sizeof(lookup));

          for(int k = 0; k < 8; k++, base += sourceStride)
          {
            SampleIterator<const byte> current(base, 0, bitsPerIndex);
            for(int l = 0; l < 8; l++, ++current)
            {
              ++(lookup[*current]);
            }
          }
          unsigned first = 0;
          unsigned second = 1;
          if(lookup[1] > lookup[0])
          {
            first = 1;
            second = 0;
          }
          for(unsigned c = 2; c < pixelMask + 1; c++)
          {
            if(lookup[c] > lookup[first])
            {
              second = first;
              first = c;
            }
            else if(lookup[c] > lookup[second])
            {
              second = c;
            }
          }
          if(lookup[second] == 0)
          {
            second = first;
          }

          targetPtr.set(first << bitsPerIndex | second);
        }
      }
    }

    void reduceColormapPairs(int bitsPerIndex, const Tile::Ptr& target, const ConstTile::Ptr& source, int x, int y)
    {
      const unsigned pixelsPerByte = 8 / bitsPerIndex;
      const unsigned pixelMask = (1U << bitsPerIndex) - 1;

      // Both source and target store two colormap indices per pixel.
      const int multiplier = 2; // two indices per pixel
      const int sourceStride = multiplier * source->width / pixelsPerByte;
      const byte* sourceBase = source->data.get();

      const int targetStride = multiplier * target->width / pixelsPerByte;
      byte* targetBase =
        target->data.get() + target->height * y * targetStride / 8 + multiplier * target->width * x / 8 / pixelsPerByte;

      for(int j = 0; j < source->height / 8; j++, targetBase += targetStride, sourceBase += sourceStride * 8)
      {
        // Iterate vertically over target
        const byte* sourcePtr = sourceBase;
        SampleIterator<uint16_t> targetPtr(reinterpret_cast<uint16_t*>(targetBase), 0, multiplier * bitsPerIndex);

        for(int i = 0; i < source->width / 8; i++, sourcePtr += 8 * multiplier / pixelsPerByte, ++targetPtr)
        {
          // Iterate horizontally over target

          // Count both indices per source pixel and retain the two most frequent.
          const byte* base = sourcePtr;
          byte lookup[pixelMask + 1];
          memset(lookup, 0, sizeof(lookup));

          for(int k = 0; k < 8; k++, base += sourceStride)
          {
            SampleIterator<const uint16_t> current(reinterpret_cast<uint16_t const*>(base), 0, multiplier * bitsPerIndex);
            for(int l = 0; l < 8; l++, ++current)
            {
              ++lookup[*current & pixelMask];
              ++lookup[*current >> bitsPerIndex];
            }
          }
          unsigned first = 0;
          unsigned second = 1;
          if(lookup[1] > lookup[0])
          {
            first = 1;
            second = 0;
          }
          for(unsigned c = 2; c < pixelMask + 1; c++)
          {
            if(lookup[c] > lookup[first])
            {
              second = first;
              first = c;
            }
            else if(lookup[c] > lookup[second])
            {
              second = c;
            }
          }
          if(lookup[second] == 0)
          {
            second = first;
          }

          targetPtr.set(first << bitsPerIndex | second);
        }
      }
    }

  } // namespace

  LayerReduction createIntensityReduction(int bitsPerSample)
  {
    require(bitsPerSample == 1 || bitsPerSample == 8 || bitsPerSample == 16);
    if(bitsPerSample == 1)
    {
      return reduce1bpp;
    }
    if(bitsPerSample == 8)
    {
      return reduce8bpp;
    }
    return reduce16bpp;
  }

  LayerReduction createRgbReduction(int bitsPerSample)
  {
    require(bitsPerSample == 8 || bitsPerSample == 16);
    if(bitsPerSample == 8)
    {
      return reduceRgb24bpp;
    }
    return reduceRgb48bpp;
  }

  LayerReduction createCmykReduction(int bitsPerSample)
  {
    require(bitsPerSample == 1 || bitsPerSample == 2 || bitsPerSample == 4 || bitsPerSample == 8 || bitsPerSample == 16);
    if(bitsPerSample == 1)
    {
      return reduceCmyk4;
    }
    if(bitsPerSample == 2)
    {
      return reduceCmyk8;
    }
    if(bitsPerSample == 4)
    {
      return reduceCmyk16;
    }
    if(bitsPerSample == 8)
    {
      return reduceCmyk32;
    }
    return reduceCmyk64;
  }

  LayerReduction createColormapReduction(int bitsPerIndex)
  {
    require(bitsPerIndex == 1 || bitsPerIndex == 2 || bitsPerIndex == 4 || bitsPerIndex == 8);
    return [bitsPerIndex](const Tile::Ptr& target, const ConstTile::Ptr& source, int x, int y)
    { reduceColormap(bitsPerIndex, target, source, x, y); };
  }

  LayerReduction createColormapPairReduction(int bitsPerIndex)
  {
    require(bitsPerIndex == 1 || bitsPerIndex == 2 || bitsPerIndex == 4 || bitsPerIndex == 8);
    return [bitsPerIndex](const Tile::Ptr& target, const ConstTile::Ptr& source, int x, int y)
    { reduceColormapPairs(bitsPerIndex, target, source, x, y); };
  }

  LayerReduction createBinaryOrReduction() { return reduceBinaryWithOr; }
} // namespace Scroom::TiledBitmap
