/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstdint>
#include <cstdlib>

#include <cairo.h>

#include <scroom/bitmap-helpers.hh>
#include <scroom/global.hh>
#include <scroom/layer-reduction.hh>
#include <scroom/layeroperations.hh>
#include <scroom/memoryblobs.hh>
#include <scroom/stuff.hh>
#include <scroom/tile.hh>

using namespace Scroom::TiledBitmap;

////////////////////////////////////////////////////////////////////////
// PipetteCommonOperationsCMYK64bpp

PipetteLayerOperations::PipetteColor
  PipetteCommonOperationsCMYK64bpp::sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr& tile)
{
  const int offset = 4 * (area.getTop() * tile->width + area.getLeft());
  const int stride = 4 * (tile->width - area.getWidth());
  auto const* sample = reinterpret_cast<uint16_t const*>(tile->data.get()) + offset;

  size_t sum_c = 0;
  size_t sum_m = 0;
  size_t sum_y = 0;
  size_t sum_k = 0;

  for(int y = area.getTop(); y < area.getBottom(); y++)
  {
    for(int x = area.getLeft(); x < area.getRight(); x++)
    {
      sum_c += sample[0];
      sum_m += sample[1];
      sum_y += sample[2];
      sum_k += sample[3];
      sample += 4;
    }
    sample += stride;
  }

  return {{"C", sum_c}, {"M", sum_m}, {"Y", sum_y}, {"K", sum_k}};
}

////////////////////////////////////////////////////////////////////////
// OperationsCMYK32

LayerOperations::Ptr OperationsCMYK32::create() { return LayerOperations::Ptr(new OperationsCMYK32()); }

OperationsCMYK32::OperationsCMYK32()
  : PipetteCommonOperationsCMYK(8)
{
}

int OperationsCMYK32::getBpp() const { return 32; }

LayerReduction OperationsCMYK32::createReduction() const { return createCmykReduction(8); }

/**
 * Cache the given tile
 */
Scroom::Utils::Stuff OperationsCMYK32::cache(const ConstTile::Ptr& tile)
{
  // Allocate the space for the cache - stride is the height of one row
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
  std::shared_ptr<uint8_t> const data = Scroom::Utils::shared_malloc(static_cast<size_t>(stride * tile->height));

  // Row is a pointer to a row of pixels (destination)
  auto* row = reinterpret_cast<uint32_t*>(data.get());
  // Cur is a pointer to the start of the row in the tile (source)
  const uint8_t* cur = tile->data.get();

  for(int i = 0; i < 4 * tile->height * tile->width; i += 4)
  {
    // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
    auto C_i = static_cast<uint8_t>(255 - cur[i]);
    auto M_i = static_cast<uint8_t>(255 - cur[i + 1]);
    auto Y_i = static_cast<uint8_t>(255 - cur[i + 2]);
    auto K_i = static_cast<uint8_t>(255 - cur[i + 3]);

    uint32_t const R = static_cast<uint8_t>((C_i * K_i) / 255);
    uint32_t const G = static_cast<uint8_t>((M_i * K_i) / 255);
    uint32_t const B = static_cast<uint8_t>((Y_i * K_i) / 255);

    // Write 255 as alpha (fully opaque)
    row[i / 4] = 255u << 24 | R << 16 | G << 8 | B;
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}


////////////////////////////////////////////////////////////////////////
// OperationsCMYK64

LayerOperations::Ptr OperationsCMYK64::create() { return LayerOperations::Ptr(new OperationsCMYK64()); }

OperationsCMYK64::OperationsCMYK64() = default;

int OperationsCMYK64::getBpp() const { return 64; }

LayerReduction OperationsCMYK64::createReduction() const { return createCmykReduction(16); }

Scroom::Utils::Stuff OperationsCMYK64::cache(const ConstTile::Ptr& tile)
{
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
  std::shared_ptr<uint8_t> const data = Scroom::Utils::shared_malloc(static_cast<size_t>(stride * tile->height));

  auto* row = reinterpret_cast<uint32_t*>(data.get());
  auto const* cur = reinterpret_cast<uint16_t const*>(tile->data.get());

  for(int i = 0; i < tile->height * tile->width; i++)
  {
    const uint8_t C_i = channel16To8(65535 - cur[4 * i]);
    const uint8_t M_i = channel16To8(65535 - cur[4 * i + 1]);
    const uint8_t Y_i = channel16To8(65535 - cur[4 * i + 2]);
    const uint8_t K_i = channel16To8(65535 - cur[4 * i + 3]);

    const uint32_t R = static_cast<uint8_t>((C_i * K_i) / 255);
    const uint32_t G = static_cast<uint8_t>((M_i * K_i) / 255);
    const uint32_t B = static_cast<uint8_t>((Y_i * K_i) / 255);

    row[i] = 255u << 24 | R << 16 | G << 8 | B;
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}


////////////////////////////////////////////////////////////////////////
// OperationsCMYK16

LayerOperations::Ptr OperationsCMYK16::create() { return LayerOperations::Ptr(new OperationsCMYK16()); }

OperationsCMYK16::OperationsCMYK16()
  : PipetteCommonOperationsCMYK(4)
{
}

int OperationsCMYK16::getBpp() const { return 16; }

LayerReduction OperationsCMYK16::createReduction() const { return createCmykReduction(4); }

/**
 * Cache the given tile
 */
Scroom::Utils::Stuff OperationsCMYK16::cache(const ConstTile::Ptr& tile)
{
  // Allocate the space for the cache - stride is the height of one row
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
  std::shared_ptr<uint8_t> const data = Scroom::Utils::shared_malloc(static_cast<size_t>(stride * tile->height));

  // Row is a pointer to a row of pixels (destination)
  auto* row = reinterpret_cast<uint32_t*>(data.get());
  // Cur is a pointer to the start of the row in the tile (source)
  const uint8_t* cur = tile->data.get();

  for(int i = 0; i < 2 * tile->height * tile->width; i += 2)
  {
    // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
    auto C_i = static_cast<uint8_t>(255 - ((cur[i]) >> 4) * 17); // 17 == 255/15
    auto M_i = static_cast<uint8_t>(255 - ((cur[i] & 0x0F)) * 17);
    auto Y_i = static_cast<uint8_t>(255 - ((cur[i + 1]) >> 4) * 17);
    auto K_i = static_cast<uint8_t>(255 - ((cur[i + 1] & 0x0F)) * 17);

    uint32_t const R = static_cast<uint8_t>((C_i * K_i) / 255);
    uint32_t const G = static_cast<uint8_t>((M_i * K_i) / 255);
    uint32_t const B = static_cast<uint8_t>((Y_i * K_i) / 255);

    // Write 255 as alpha (fully opaque)
    row[i / 2] = 255u << 24 | R << 16 | G << 8 | B;
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}


////////////////////////////////////////////////////////////////////////
// OperationsCMYK8

LayerOperations::Ptr OperationsCMYK8::create() { return LayerOperations::Ptr(new OperationsCMYK8()); }

OperationsCMYK8::OperationsCMYK8()
  : PipetteCommonOperationsCMYK(2)
{
}

int OperationsCMYK8::getBpp() const { return 8; }

LayerReduction OperationsCMYK8::createReduction() const { return createCmykReduction(2); }

/**
 * Cache the given tile
 */
Scroom::Utils::Stuff OperationsCMYK8::cache(const ConstTile::Ptr& tile)
{
  // Allocate the space for the cache - stride is the height of one row
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
  std::shared_ptr<uint8_t> const data = Scroom::Utils::shared_malloc(static_cast<size_t>(stride * tile->height));

  // Row is a pointer to a row of pixels (destination)
  auto* row = reinterpret_cast<uint32_t*>(data.get());
  // Cur is a pointer to the start of the row in the tile (source)
  const uint8_t* cur = tile->data.get();

  // assume stride = tile->width * 4
  for(int i = 0; i < tile->height * tile->width; i++)
  {
    // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
    auto C_i = static_cast<uint8_t>(255 - ((cur[i]) >> 6) * 85); // 85 == 255/3
    auto M_i = static_cast<uint8_t>(255 - ((cur[i] & 0x30) >> 4) * 85);
    auto Y_i = static_cast<uint8_t>(255 - ((cur[i] & 0x0C) >> 2) * 85);
    auto K_i = static_cast<uint8_t>(255 - ((cur[i] & 0x03)) * 85);

    uint32_t const R = static_cast<uint8_t>((C_i * K_i) / 255);
    uint32_t const G = static_cast<uint8_t>((M_i * K_i) / 255);
    uint32_t const B = static_cast<uint8_t>((Y_i * K_i) / 255);

    // Write 255 as alpha (fully opaque)
    row[i] = 255u << 24 | R << 16 | G << 8 | B;
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}


////////////////////////////////////////////////////////////////////////
// OperationsCMYK4

LayerOperations::Ptr OperationsCMYK4::create() { return LayerOperations::Ptr(new OperationsCMYK4()); }

OperationsCMYK4::OperationsCMYK4()
  : PipetteCommonOperationsCMYK(1)
{
}

int OperationsCMYK4::getBpp() const { return 4; }

LayerReduction OperationsCMYK4::createReduction() const { return createCmykReduction(1); }

/**
 * Cache the given tile
 */
Scroom::Utils::Stuff OperationsCMYK4::cache(const ConstTile::Ptr& tile)
{
  // Allocate the space for the cache - stride is the height of one row
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, tile->width);
  std::shared_ptr<uint8_t> const data = Scroom::Utils::shared_malloc(static_cast<size_t>(stride * tile->height));

  // Row is a pointer to a row of pixels (destination)
  auto* row = reinterpret_cast<uint32_t*>(data.get());
  // Cur is a pointer to the start of the row in the tile (source)
  const uint8_t* cur = tile->data.get();

  // assume stride = tile->width * 4
  for(int i = 0; i < tile->height * tile->width; i++)
  {
    // Convert CMYK to ARGB, because cairo doesn't know how to render CMYK.
    uint8_t C_i;

    uint8_t M_i;

    uint8_t Y_i;

    uint8_t K_i;
    if((i & 1) == 0)
    { // even pixels -> top half of byte
      C_i = static_cast<uint8_t>(((cur[i / 2]) >> 7) - 1); // 0 -> 255 (= -1), 1 -> 0
      M_i = static_cast<uint8_t>(((cur[i / 2] & 0x40) >> 6) - 1);
      Y_i = static_cast<uint8_t>(((cur[i / 2] & 0x20) >> 5) - 1);
      K_i = static_cast<uint8_t>(((cur[i / 2] & 0x10) >> 4) - 1);
    }
    else
    { // odd pixels -> lower half of the byte
      C_i = static_cast<uint8_t>(((cur[i / 2] & 0x08) >> 3) - 1); // 0 -> 255 (= -1), 1 -> 0
      M_i = static_cast<uint8_t>(((cur[i / 2] & 0x04) >> 2) - 1);
      Y_i = static_cast<uint8_t>(((cur[i / 2] & 0x02) >> 1) - 1);
      K_i = static_cast<uint8_t>(((cur[i / 2] & 0x01)) - 1);
    }

    uint32_t const R = static_cast<uint8_t>((C_i * K_i) / 255);
    uint32_t const G = static_cast<uint8_t>((M_i * K_i) / 255);
    uint32_t const B = static_cast<uint8_t>((Y_i * K_i) / 255);

    // Write 255 as alpha (fully opaque)
    row[i] = 255u << 24 | R << 16 | G << 8 | B;
  }

  return Scroom::Bitmap::BitmapSurface::create(tile->width, tile->height, CAIRO_FORMAT_ARGB32, stride, data);
}
