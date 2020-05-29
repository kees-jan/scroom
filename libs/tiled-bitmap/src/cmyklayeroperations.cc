/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/bitmap-helpers.hh>
#include <scroom/layeroperations.hh>

////////////////////////////////////////////////////////////////////////

namespace
{
  boost::shared_ptr<unsigned char> shared_malloc(size_t size)
  {
    return boost::shared_ptr<unsigned char>(static_cast<unsigned char*>(malloc(size)), free);
  }
}

////////////////////////////////////////////////////////////////////////
// OperationsCMYK32

LayerOperations::Ptr OperationsCMYK32::create()
{
  return Ptr(new OperationsCMYK32());
}

OperationsCMYK32::OperationsCMYK32()
{
}

int OperationsCMYK32::getBpp()
{
  return 32;
}

/**
 * Cache the given tile
 */
Scroom::Utils::Stuff OperationsCMYK32::cache(const ConstTile::Ptr tile)
{
  // Allocate the space for the cache - stride is the height of one row
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
  boost::shared_ptr<uint8_t> data = shared_malloc(static_cast<size_t>(stride * tile->height));

  // Row is a pointer to a row of pixels (destination)
  uint32_t* row = reinterpret_cast<uint32_t*>(data.get());
  // Cur is a pointer to the start of the row in the tile (source)
  const uint8_t* cur = tile->data.get();

  for (int i = 0; i < 4 * tile->height * tile->width; i += 4)
  {
    // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
    uint8_t C_i = static_cast<uint8_t>(255 - cur[i    ]);
    uint8_t M_i = static_cast<uint8_t>(255 - cur[i + 1]);
    uint8_t Y_i = static_cast<uint8_t>(255 - cur[i + 2]);
    uint8_t K_i = static_cast<uint8_t>(255 - cur[i + 3]);

    uint32_t R = static_cast<uint8_t>((C_i * K_i) / 255);
    uint32_t G = static_cast<uint8_t>((M_i * K_i) / 255);
    uint32_t B = static_cast<uint8_t>((Y_i * K_i) / 255);

    // Write 255 as alpha (fully opaque)
    row[i / 4] = 255u << 24 | R << 16 | G << 8 | B;
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}

void OperationsCMYK32::reduce(Tile::Ptr target, const ConstTile::Ptr source, int top_left_x, int top_left_y)
{
  // Reducing by a factor 8
  int sourceStride = 8 * source->width / 2; // stride in bytes
  const byte* sourceBase = source->data.get();

  int targetStride = 8 * target->width / 2; // stride in bytes
  byte* targetBase = target->data.get() +
      (target->height * top_left_y + top_left_x) * targetStride / 8;

  for (int y = 0; y < source->height / 8; y++)
  {
    byte* targetPtr = targetBase;

    for (int x = 0; x < source->width / 8; x++)
    {
      // We want to store the average colour of the 8*8 pixel image
      // with (x, y) as its top-left corner into targetPtr.
      const byte* base = sourceBase + 8 * 4 * x; // start of the row
      const byte* end = base + 8 * sourceStride; // end of the row

      int sum_c = 0;
      int sum_m = 0;
      int sum_y = 0;
      int sum_k = 0;
      for (const byte* row = base; row < end; row += sourceStride)
      {
        for (size_t current = 0; current < 8 * 4; current += 4)
        {
          sum_c += row[current    ];
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

////////////////////////////////////////////////////////////////////////
// OperationsCMYK16

LayerOperations::Ptr OperationsCMYK16::create()
{
  return Ptr(new OperationsCMYK16());
}

OperationsCMYK16::OperationsCMYK16()
{
}

int OperationsCMYK16::getBpp()
{
  return 16;
}

/**
 * Cache the given tile
 */
Scroom::Utils::Stuff OperationsCMYK16::cache(const ConstTile::Ptr tile)
{
  // Allocate the space for the cache - stride is the height of one row
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
  boost::shared_ptr<uint8_t> data = shared_malloc(static_cast<size_t>(stride * tile->height));

  // Row is a pointer to a row of pixels (destination)
  uint32_t* row = reinterpret_cast<uint32_t*>(data.get());
  // Cur is a pointer to the start of the row in the tile (source)
  const uint8_t* cur = tile->data.get();

  for (int i = 0; i < 2 * tile->height * tile->width; i += 2)
  {
    // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
    uint8_t C_i = static_cast<uint8_t>(255 - ((cur[i    ]       ) >> 4) * 17); // 17 == 255/15
    uint8_t M_i = static_cast<uint8_t>(255 - ((cur[i    ] & 0x0F)     ) * 17);
    uint8_t Y_i = static_cast<uint8_t>(255 - ((cur[i + 1]       ) >> 4) * 17);
    uint8_t K_i = static_cast<uint8_t>(255 - ((cur[i + 1] & 0x0F)     ) * 17);

    uint32_t R = static_cast<uint8_t>((C_i * K_i) / 255);
    uint32_t G = static_cast<uint8_t>((M_i * K_i) / 255);
    uint32_t B = static_cast<uint8_t>((Y_i * K_i) / 255);

    // Write 255 as alpha (fully opaque)
    row[i / 2] = 255u << 24 | R << 16 | G << 8 | B;
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}

void OperationsCMYK16::reduce(Tile::Ptr target, const ConstTile::Ptr source, int top_left_x, int top_left_y)
{
  // Reducing by a factor 8
  int sourceStride = 4 * source->width / 2; // stride in bytes
  const byte* sourceBase = source->data.get();

  int targetStride = 8 * target->width / 2; // stride in bytes
  byte* targetBase = target->data.get() +
      (target->height * top_left_y + top_left_x) * targetStride / 8;

  for (int y = 0; y < source->height / 8; y++)
  {
    byte* targetPtr = targetBase;

    for (int x = 0; x < source->width / 8; x++)
    {
      // We want to store the average colour of the 8*8 pixel image
      // with (x, y) as its top-left corner into targetPtr.
      const byte* base = sourceBase + 4 * 4 * x; // start of the row
      const byte* end = base + 8 * sourceStride; // end of the row

      int sum_c = 0;
      int sum_m = 0;
      int sum_y = 0;
      int sum_k = 0;
      for (const byte* row = base; row < end; row += sourceStride)
      {
        for (size_t current = 0; current < 8 * 2; current += 2)
        {
          sum_c += row[current    ] >> 4;
          sum_m += row[current    ] & 15;
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

////////////////////////////////////////////////////////////////////////
// OperationsCMYK8

LayerOperations::Ptr OperationsCMYK8::create()
{
  return Ptr(new OperationsCMYK8());
}

OperationsCMYK8::OperationsCMYK8()
{
}

int OperationsCMYK8::getBpp()
{
  return 8;
}

/**
 * Cache the given tile
 */
Scroom::Utils::Stuff OperationsCMYK8::cache(const ConstTile::Ptr tile)
{
  // Allocate the space for the cache - stride is the height of one row
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
  boost::shared_ptr<uint8_t> data = shared_malloc(static_cast<size_t>(stride * tile->height));

  // Row is a pointer to a row of pixels (destination)
  uint32_t* row = reinterpret_cast<uint32_t*>(data.get());
  // Cur is a pointer to the start of the row in the tile (source)
  const uint8_t* cur = tile->data.get();

  // assume stride = tile->width * 4
  for (int i = 0; i < tile->height * tile->width; i++)
  {
    // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
    uint8_t C_i = static_cast<uint8_t>(255 - ((cur[i]       ) >> 6) * 85); // 85 == 255/3
    uint8_t M_i = static_cast<uint8_t>(255 - ((cur[i] & 0x30) >> 4) * 85);
    uint8_t Y_i = static_cast<uint8_t>(255 - ((cur[i] & 0x0C) >> 2) * 85);
    uint8_t K_i = static_cast<uint8_t>(255 - ((cur[i] & 0x03)     ) * 85);

    uint32_t R = static_cast<uint8_t>((C_i * K_i) / 255);
    uint32_t G = static_cast<uint8_t>((M_i * K_i) / 255);
    uint32_t B = static_cast<uint8_t>((Y_i * K_i) / 255);

    // Write 255 as alpha (fully opaque)
    row[i] = 255u << 24 | R << 16 | G << 8 | B;
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}

void OperationsCMYK8::reduce(Tile::Ptr target, const ConstTile::Ptr source, int top_left_x, int top_left_y)
{
  // Reducing by a factor 8
  int sourceStride = source->width; // stride in bytes
  const byte* sourceBase = source->data.get();

  int targetStride = 8 * target->width / 2; // stride in bytes
  byte* targetBase = target->data.get() +
      (target->height * top_left_y + top_left_x) * targetStride / 8;
  
  for (int y = 0; y < source->height / 8; y++)
  {
    for (int x = 0; x < source->width / 8; x++)
    {
      // We want to store the average colour of the 8*8 pixel image
      // with (x, y) as its top-left corner into targetPtr.
      const byte* base = sourceBase + 2 * 4 * x; // start of the row
      const byte* end = base + 8 * sourceStride; // end of the row

      int sum_c = 0;
      int sum_m = 0;
      int sum_y = 0;
      int sum_k = 0;
      for (const byte* row = base; row < end; row += sourceStride)
      {
        for (size_t current = 0; current < 8; current++)
        {
          sum_c +=  row[current]         >> 6;
          sum_m += (row[current] & 0x30) >> 4;
          sum_y += (row[current] & 0x0C) >> 2;
          sum_k +=  row[current] & 0x03;
        }
      }

      targetBase[4*x + 0] = static_cast<byte>(sum_c * 255 / 192);
      targetBase[4*x + 1] = static_cast<byte>(sum_m * 255 / 192);
      targetBase[4*x + 2] = static_cast<byte>(sum_y * 255 / 192);
      targetBase[4*x + 3] = static_cast<byte>(sum_k * 255 / 192);
    }

    targetBase += targetStride;
    sourceBase += sourceStride * 8;
  }
}

////////////////////////////////////////////////////////////////////////
// OperationsCMYK4

LayerOperations::Ptr OperationsCMYK4::create()
{
  return Ptr(new OperationsCMYK4());
}

OperationsCMYK4::OperationsCMYK4()
{
}

int OperationsCMYK4::getBpp()
{
  // CMYK has 4 channels -> 4 samples per pixel.
  return 4;
}

/**
 * Cache the given tile
 */
Scroom::Utils::Stuff OperationsCMYK4::cache(const ConstTile::Ptr tile)
{
  // Allocate the space for the cache - stride is the height of one row
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
  boost::shared_ptr<uint8_t> data = shared_malloc(static_cast<size_t>(stride * tile->height));

  // Row is a pointer to a row of pixels (destination)
  uint32_t* row = reinterpret_cast<uint32_t*>(data.get());
  // Cur is a pointer to the start of the row in the tile (source)
  const uint8_t* cur = tile->data.get();

  // assume stride = tile->width * 4
  for (int i = 0; i < tile->height * tile->width; i++)
  {
    // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
    uint8_t C_i, M_i, Y_i, K_i;
    if ((i & 1) == 0) { // even pixels -> top half of byte
      C_i = static_cast<uint8_t>(((cur[i / 2]       ) >> 7) - 1); // 0 -> 255 (= -1), 1 -> 0
      M_i = static_cast<uint8_t>(((cur[i / 2] & 0x40) >> 6) - 1);
      Y_i = static_cast<uint8_t>(((cur[i / 2] & 0x20) >> 5) - 1);
      K_i = static_cast<uint8_t>(((cur[i / 2] & 0x10) >> 4) - 1);
    } else { // odd pixels -> lower half of the byte
      C_i = static_cast<uint8_t>(((cur[i / 2] & 0x08) >> 3) - 1); // 0 -> 255 (= -1), 1 -> 0
      M_i = static_cast<uint8_t>(((cur[i / 2] & 0x04) >> 2) - 1);
      Y_i = static_cast<uint8_t>(((cur[i / 2] & 0x02) >> 1) - 1);
      K_i = static_cast<uint8_t>(((cur[i / 2] & 0x01)     ) - 1);
    }

    uint32_t R = static_cast<uint8_t>((C_i * K_i) / 255);
    uint32_t G = static_cast<uint8_t>((M_i * K_i) / 255);
    uint32_t B = static_cast<uint8_t>((Y_i * K_i) / 255);

    // Write 255 as alpha (fully opaque)
    row[i] = 255u << 24 | R << 16 | G << 8 | B;
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}

void OperationsCMYK4::reduce(Tile::Ptr target, const ConstTile::Ptr source, int top_left_x, int top_left_y)
{
  // Reducing by a factor 8
  int sourceStride = source->width / 2; // stride in bytes
  const byte* sourceBase = source->data.get();

  int targetStride = 8 * target->width / 2; // stride in bytes
  byte* targetBase = target->data.get() +
      (target->height * top_left_y + top_left_x) * targetStride / 8;

  for (int y = 0; y < source->height / 8; y++)
  {
    for (int x = 0; x < source->width / 8; x++)
    {
      // We want to store the average colour of the 8*8 pixel image
      // with (x, y) as its top-left corner into targetPtr.
      const byte* base = sourceBase + 4 * x; // start of the row
      const byte* end = base + 8 * sourceStride; // end of the row

      int sum_c = 0;
      int sum_m = 0;
      int sum_y = 0;
      int sum_k = 0;
      for (const byte* row = base; row < end; row += sourceStride)
      {
        for (size_t current = 0; current < 4; current++)
        {
          sum_c += ((row[current] >> 7) & 1) + ((row[current] >> 3) & 1);
          sum_m += ((row[current] >> 6) & 1) + ((row[current] >> 2) & 1);
          sum_y += ((row[current] >> 5) & 1) + ((row[current] >> 1) & 1);
          sum_k += ((row[current] >> 4) & 1) + ((row[current] >> 0) & 1);
        }
      }

      targetBase[4*x  ] = static_cast<uint8_t>(sum_c * 255 / 64);
      targetBase[4*x+1] = static_cast<uint8_t>(sum_m * 255 / 64);
      targetBase[4*x+2] = static_cast<uint8_t>(sum_y * 255 / 64);
      targetBase[4*x+3] = static_cast<uint8_t>(sum_k * 255 / 64);
    }

    targetBase += targetStride;
    sourceBase += sourceStride * 8;
  }
}
