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
    namespace
    {
      const char* to_string(bool b) { return b ? "true" : "false"; }
    } // namespace

    std::ostream& to_stream(std::ostream& os, const BitmapMetaData& bmd)
    {
      os << "Type=\"" << bmd.type << "\", rect=" << bmd.rect << ", spp=" << bmd.samplesPerPixel << ", bps=" << bmd.bitsPerSample
         << ", colormapped=" << to_string(static_cast<bool>(bmd.colormapHelper));

      return os;
    }

    std::string to_string(const BitmapMetaData& bmd)
    {
      std::stringstream ss;
      to_stream(ss, bmd);
      return ss.str();
    }


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
        printf("ERROR: RGB bitmaps with %u bits per sample are not supported\n", bmd.bitsPerSample);
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
        ls.push_back(OperationsCMYK32::create());
      }
      else if(bmd.bitsPerSample == 2)
      {
        ls.push_back(OperationsCMYK8::create());
        ls.push_back(OperationsCMYK32::create());
      }
      else if(bmd.bitsPerSample == 1)
      {
        ls.push_back(OperationsCMYK4::create());
        ls.push_back(OperationsCMYK32::create());
      }
      else
      {
        printf("ERROR: CMYK bitmaps with %u bits per sample are not supported\n", bmd.bitsPerSample);
        return {};
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

      if(bmd.bitsPerSample != 1 && bmd.bitsPerSample != 2 && bmd.bitsPerSample != 4 && bmd.bitsPerSample != 8)
      {
        printf("ERROR: Greyscale bitmaps with %u bits per pixel are not supported (yet)\n", bmd.bitsPerSample);
        return {};
      }

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

      if(bmd.bitsPerSample != 1 && bmd.bitsPerSample != 2 && bmd.bitsPerSample != 4 && bmd.bitsPerSample != 8)
      {
        printf("ERROR: Colormapped bitmaps with %u bits per pixel are not supported (yet)\n", bmd.bitsPerSample);
        return {};
      }

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
