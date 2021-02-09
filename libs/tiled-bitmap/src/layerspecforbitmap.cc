/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/range/adaptor/reversed.hpp>

#include <scroom/layeroperations.hh>
#include <scroom/opentiledbitmapinterface.hh>

namespace Scroom
{
  namespace TiledBitmap
  {
    const std::string RGB         = "RGB";
    const std::string CMYK        = "CMYK";
    const std::string Greyscale   = "Greyscale";
    const std::string Colormapped = "Colormapped";

    LayerSpecResult RGBBitmap(const BitmapMetaData& bmd);
    LayerSpecResult CMYKBitmap(const BitmapMetaData& bmd);
    LayerSpecResult GreyscaleBitmap(const BitmapMetaData& bmd);
    LayerSpecResult ColormappedBitmap(const BitmapMetaData& bmd);

    ///////////////////////////////////////////////////////////////////

    class LayerSpecForBitmapper
    {
    public:
      using Ptr = boost::shared_ptr<LayerSpecForBitmapper>;

      static Ptr instance()
      {
        static Ptr lsfb(new LayerSpecForBitmapper);
        return lsfb;
      }

      void registerFunction(LayerSpecForBitmapFunc f) { functions.push_back(f); }

      LayerSpecResult get(const BitmapMetaData& bmd)
      {
        LayerSpecResult ls;

        // Later registered functions override earlier ones
        for(const auto& f: boost::adaptors::reverse(functions))
        {
          if(!std::get<0>(ls).empty())
          {
            break;
          }

          ls = f(bmd);
        }

        return ls;
      }

    private:
      LayerSpecForBitmapper()
        : functions({RGBBitmap, CMYKBitmap, GreyscaleBitmap, ColormappedBitmap})
      {}

      std::vector<LayerSpecForBitmapFunc> functions;
    };

    ///////////////////////////////////////////////////////////////////

    LayerSpecResult RGBBitmap(const BitmapMetaData& bmd)
    {
      if(bmd.type != RGB)
      {
        return {};
      };

      require(bmd.samplesPerPixel == 3);
      require(!bmd.colormapHelper);

      if(bmd.bitsPerSample != 8)
      {
        printf("PANIC: Bits per sample is not 8, but %d. Giving up\n", bmd.bitsPerSample);
        return {};
      }

      return LayerSpecResult({Operations24bpp::create()}, nullptr);
    }

    LayerSpecResult CMYKBitmap(const BitmapMetaData& bmd)
    {
      if(bmd.type != CMYK)
      {
        return {};
      }

      require(bmd.samplesPerPixel == 4);
      require(!bmd.colormapHelper);

      LayerSpec ls;

      if(bmd.bitsPerSample == 8)
      {
        ls.push_back(OperationsCMYK32::create());
      }
      else if(bmd.bitsPerSample == 4)
      {
        ls.push_back(OperationsCMYK16::create());
      }
      else if(bmd.bitsPerSample == 2)
      {
        ls.push_back(OperationsCMYK8::create());
      }
      else if(bmd.bitsPerSample == 1)
      {
        ls.push_back(OperationsCMYK4::create());
      }

      return LayerSpecResult(ls, nullptr);
    }

    LayerSpecResult GreyscaleBitmap(const BitmapMetaData& bmd)
    {
      if(bmd.type != Greyscale)
      {
        return {};
      }

      require(bmd.samplesPerPixel == 1);

      LayerSpec               ls;
      ColormapHelperBase::Ptr colormapHelper = bmd.colormapHelper;
      if(!colormapHelper)
      {
        colormapHelper = MonochromeColormapHelper::create(2);
      }

      if(bmd.bitsPerSample == 2 || bmd.bitsPerSample == 4)
      {
        ls.push_back(Operations::create(colormapHelper, bmd.bitsPerSample));
        ls.push_back(OperationsColormapped::create(colormapHelper, bmd.bitsPerSample));
      }
      else
      {
        if(bmd.bitsPerSample == 1)
        {
          ls.push_back(Operations1bpp::create(colormapHelper));
        }
        ls.push_back(Operations8bpp::create(colormapHelper));
      }

      return LayerSpecResult(ls, colormapHelper);
    }

    LayerSpecResult ColormappedBitmap(const BitmapMetaData& bmd)
    {
      if(bmd.type != Colormapped)
      {
        return {};
      }
      require(bmd.samplesPerPixel == 1);
      require(bmd.colormapHelper);

      LayerSpec ls;
      ls.push_back(Operations::create(bmd.colormapHelper, bmd.bitsPerSample));
      ls.push_back(OperationsColormapped::create(bmd.colormapHelper, bmd.bitsPerSample));

      return LayerSpecResult(ls, bmd.colormapHelper);
    }

    ///////////////////////////////////////////////////////////////////
    LayerSpecResult LayerSpecForBitmap(const BitmapMetaData& bitmapMetaData)
    {
      return LayerSpecForBitmapper::instance()->get(bitmapMetaData);
    }

  } // namespace TiledBitmap
} // namespace Scroom
