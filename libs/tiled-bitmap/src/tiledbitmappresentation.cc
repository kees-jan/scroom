/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <utility>

#include <scroom/cairo-helpers.hh>
#include <scroom/opentiledbitmapinterface.hh>
#include <scroom/pipetteviewinterface.hh>
#include <scroom/transformpresentation.hh>

namespace
{
  using namespace Scroom::TiledBitmap;

  /**
   * Add two pipette color map values of the same key.
   */
  PipetteLayerOperations::PipetteColor sumPipetteColors(const PipetteLayerOperations::PipetteColor& lhs,
                                                        const PipetteLayerOperations::PipetteColor& rhs)
  {
    PipetteLayerOperations::PipetteColor result;
    if(lhs.empty())
    {
      return rhs;
    }
    for(unsigned int i = 0; i < rhs.size(); i++)
    {
      result.push_back({rhs[i].first, rhs[i].second + lhs[i].second});
    }
    return result;
  }

  /**
   * Divides each element inside elements by by a constant divisor.
   */
  PipetteLayerOperations::PipetteColor dividePipetteColors(PipetteLayerOperations::PipetteColor elements, const int divisor)
  {
    for(auto& elem: elements)
    {
      elem.second /= divisor;
    }
    return elements;
  }

  class TiledBitmapPresentation
    : public PresentationBase
    , public Colormappable
    , public PipetteViewInterface
  {
  public:
    using Ptr = boost::shared_ptr<TiledBitmapPresentation>;

  private:
    using Views = std::set<ViewInterface::WeakPtr>;

    std::string                        name;
    Scroom::Utils::Rectangle<int>      rect;
    TiledBitmapInterface::Ptr          tbi;
    std::map<std::string, std::string> properties;
    Views                              views;
    ColormapHelper::Ptr                colormapHelper;
    PipetteLayerOperations::Ptr        pipetteLayerOperation;

  public:
    static TiledBitmapPresentation::Ptr create(std::string                        name_,
                                               Scroom::Utils::Rectangle<int>      rect_,
                                               TiledBitmapInterface::Ptr          tbi_,
                                               std::map<std::string, std::string> properties_,
                                               Views                              views_,
                                               ColormapHelper::Ptr                colormapHelper_,
                                               PipetteLayerOperations::Ptr        pipetteLayerOperation_)
    {
      return Ptr(new TiledBitmapPresentation(std::move(name_),
                                             std::move(rect_),
                                             std::move(tbi_),
                                             std::move(properties_),
                                             std::move(views_),
                                             std::move(colormapHelper_),
                                             std::move(pipetteLayerOperation_)));
    }

    ////////////////////////////////////////////////////////////////////////
    // PresentationInterface
    ////////////////////////////////////////////////////////////////////////

    Scroom::Utils::Rectangle<double> getRect() override;
    void redraw(const ViewInterface::Ptr& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom) override;
    bool getProperty(const std::string& propertyName, std::string& value) override;
    bool isPropertyDefined(const std::string& propertyName) override;
    std::string getTitle() override;

    ////////////////////////////////////////////////////////////////////////
    // PipetteViewInterface
    ////////////////////////////////////////////////////////////////////////

    PipetteLayerOperations::PipetteColor getPixelAverages(Scroom::Utils::Rectangle<int> area) override;

    ////////////////////////////////////////////////////////////////////////
    // Colormappable
    ////////////////////////////////////////////////////////////////////////

    void          setColormap(Colormap::Ptr colormap) override;
    Colormap::Ptr getOriginalColormap() override;
    int           getNumberOfColors() override;
    Color         getMonochromeColor() override;
    void          setMonochromeColor(const Color& c) override;
    void          setTransparentBackground() override;
    void          disableTransparentBackground() override;
    bool          getTransparentBackground() override;

  protected:
    ////////////////////////////////////////////////////////////////////////
    // PresentationBase
    ////////////////////////////////////////////////////////////////////////
    void                             viewAdded(ViewInterface::WeakPtr vi) override;
    void                             viewRemoved(ViewInterface::WeakPtr vi) override;
    std::set<ViewInterface::WeakPtr> getViews() override;

  private:
    void clearCaches();
    TiledBitmapPresentation(std::string&&                        name_,
                            Scroom::Utils::Rectangle<int>&&      rect_,
                            TiledBitmapInterface::Ptr&&          tbi_,
                            std::map<std::string, std::string>&& properties_,
                            Views&&                              views_,
                            ColormapHelper::Ptr&&                colormapHelper_,
                            PipetteLayerOperations::Ptr&&        pipetteLayerOperation_)
      : name(name_)
      , rect(rect_)
      , tbi(tbi_)
      , properties(properties_)
      , views(views_)
      , colormapHelper(colormapHelper_)
      , pipetteLayerOperation(pipetteLayerOperation_)
    {}
  };

  void TiledBitmapPresentation::clearCaches()
  {
    for(const Views::value_type& p: views)
    {
      ViewInterface::Ptr v = p.lock();
      if(v)
      {
        if(tbi)
        {
          tbi->clearCaches(v);
        }
        v->invalidate();
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////
  // PresentationInterface
  ////////////////////////////////////////////////////////////////////////

  Scroom::Utils::Rectangle<double> TiledBitmapPresentation::getRect() { return rect; }

  void TiledBitmapPresentation::redraw(const ViewInterface::Ptr&        vi,
                                       cairo_t*                         cr,
                                       Scroom::Utils::Rectangle<double> presentationArea,
                                       int                              zoom)
  {
    drawOutOfBoundsWithoutBackground(cr, presentationArea, getRect(), pixelSizeFromZoom(zoom));

    if(tbi)
    {
      tbi->redraw(vi, cr, presentationArea, zoom);
    }
  }

  bool TiledBitmapPresentation::getProperty(const std::string& propertyName, std::string& value)
  {
    auto p     = properties.find(propertyName);
    bool found = false;
    if(p == properties.end())
    {
      found = false;
      value = "";
    }
    else
    {
      found = true;
      value = p->second;
    }

    return found;
  }

  bool TiledBitmapPresentation::isPropertyDefined(const std::string& propertyName)
  {
    return properties.end() != properties.find(propertyName);
  }

  std::string TiledBitmapPresentation::getTitle() { return name; }

  ////////////////////////////////////////////////////////////////////////
  // PipetteViewInterface
  ////////////////////////////////////////////////////////////////////////

  PipetteLayerOperations::PipetteColor TiledBitmapPresentation::getPixelAverages(Scroom::Utils::Rectangle<int> area)
  {
    require(pipetteLayerOperation);

    Scroom::Utils::Rectangle<int> presentationArea = getRect().toIntRectangle();
    area                                           = area.intersection(presentationArea);

    Layer::Ptr                           bottomLayer = tbi->getBottomLayer();
    PipetteLayerOperations::PipetteColor pipetteColors;

    int totalPixels = area.getWidth() * area.getHeight();

    if(totalPixels == 0)
    {
      return {};
    }

    // Get start tile (tile_pos_x_start, tile_pos_y_start)
    int tile_pos_x_start = area.getLeft() / TILESIZE;
    int tile_pos_y_start = area.getTop() / TILESIZE;

    // Get end tile (tile_pos_x_end, tile_pos_y_end)
    int tile_pos_x_end = (area.getRight() - 1) / TILESIZE;
    int tile_pos_y_end = (area.getBottom() - 1) / TILESIZE;

    for(int x = tile_pos_x_start; x <= tile_pos_x_end; x++)
    {
      for(int y = tile_pos_y_start; y <= tile_pos_y_end; y++)
      {
        ConstTile::Ptr                tile = bottomLayer->getTile(x, y)->getConstTileSync();
        Scroom::Utils::Rectangle<int> tile_rectangle(x * TILESIZE, y * TILESIZE, tile->width, tile->height);

        Scroom::Utils::Rectangle<int> inter_rect = tile_rectangle.intersection(area);
        Scroom::Utils::Point<int>     base(x * TILESIZE, y * TILESIZE);

        inter_rect -= base; // rectangle coordinates relative to constTile with topleft corner (0,0)

        pipetteColors = sumPipetteColors(pipetteColors, pipetteLayerOperation->sumPixelValues(inter_rect, tile));
      }
    }
    return dividePipetteColors(pipetteColors, totalPixels);
  }

  ////////////////////////////////////////////////////////////////////////
  // PresentationBase
  ////////////////////////////////////////////////////////////////////////

  void TiledBitmapPresentation::viewAdded(ViewInterface::WeakPtr viewInterface)
  {
    views.insert(viewInterface);

    if(tbi)
    {
      tbi->open(viewInterface);
    }
    else
    {
      printf("ERROR: TiffPresentation::open(): No TiledBitmapInterface available!\n");
    }
  }

  void TiledBitmapPresentation::viewRemoved(ViewInterface::WeakPtr vi)
  {
    views.erase(vi);

    if(tbi)
    {
      tbi->close(vi);
    }
    else
    {
      printf("ERROR: TiffPresentation::close(): No TiledBitmapInterface available!\n");
    }
  }

  std::set<ViewInterface::WeakPtr> TiledBitmapPresentation::getViews() { return views; }

  ////////////////////////////////////////////////////////////////////////
  // Colormappable
  ////////////////////////////////////////////////////////////////////////

  void TiledBitmapPresentation::setColormap(Colormap::Ptr colormap)
  {
    colormapHelper->setColormap(colormap);
    clearCaches();
  }

  Colormap::Ptr TiledBitmapPresentation::getOriginalColormap() { return colormapHelper->getOriginalColormap(); }
  int           TiledBitmapPresentation::getNumberOfColors() { return colormapHelper->getNumberOfColors(); }
  Color         TiledBitmapPresentation::getMonochromeColor() { return colormapHelper->getMonochromeColor(); }

  void TiledBitmapPresentation::setMonochromeColor(const Color& c)
  {
    colormapHelper->setMonochromeColor(c);
    clearCaches();
  }

  void TiledBitmapPresentation::setTransparentBackground()
  {
    colormapHelper->setTransparentBackground();
    clearCaches();
  }

  void TiledBitmapPresentation::disableTransparentBackground()
  {
    colormapHelper->disableTransparentBackground();
    clearCaches();
  }

  bool TiledBitmapPresentation::getTransparentBackground() { return colormapHelper->getTransparentBackground(); }

  // OpenTiledBitmapAsPresentation ////////////////////////////////////
  class OpenTiledBitmapAsPresentation : public OpenPresentationInterface
  {
  public:
    using Ptr = boost::shared_ptr<OpenTiledBitmapAsPresentation>;

  private:
    OpenTiledBitmapInterface::Ptr openTiledBitmapInterface;

  public:
    static Ptr create(OpenTiledBitmapInterface::Ptr openTiledBitmapInterface_)
    {
      return Ptr(new OpenTiledBitmapAsPresentation(std::move(openTiledBitmapInterface_)));
    }

    // OpenPresentationInterface
    std::list<GtkFileFilter*> getFilters() override { return openTiledBitmapInterface->getFilters(); }

    PresentationInterface::Ptr open(const std::string& fileName) override;

  private:
    explicit OpenTiledBitmapAsPresentation(OpenTiledBitmapInterface::Ptr openTiledBitmapInterface_)
      : openTiledBitmapInterface(std::move(openTiledBitmapInterface_))
    {}
  };

  PresentationInterface::Ptr OpenTiledBitmapAsPresentation::open(const std::string& fileName)
  {
    auto           t     = openTiledBitmapInterface->open(fileName);
    BitmapMetaData bmd   = std::move(std::get<0>(t));
    Layer::Ptr     layer = std::move(std::get<1>(t));
    ReloadFunction load  = std::move(std::get<2>(t));

    // Todo... add stuff...
    return TiledBitmapPresentation::Ptr();
  }
} // namespace

namespace Scroom
{
  namespace TiledBitmap
  {
    OpenPresentationInterface::Ptr ToOpenPresentationInterface(OpenTiledBitmapInterface::Ptr openTiledBitmapInterface)
    {
      return OpenTiledBitmapAsPresentation::create(std::move(openTiledBitmapInterface));
    }
  } // namespace TiledBitmap
} // namespace Scroom
