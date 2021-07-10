/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <sstream>
#include <utility>

#include <scroom/cairo-helpers.hh>
#include <scroom/imagemdinterface.hh>
#include <scroom/opentiledbitmapinterface.hh>
#include <scroom/pipetteviewinterface.hh>
#include <scroom/transformpresentation.hh>

#include "tiled-bitmap.hh"


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

    std::string                                                name;
    Scroom::Utils::Rectangle<int>                              rect;
    TiledBitmapInterface::Ptr                                  tbi;
    std::map<std::string, std::string>                         properties;
    Views                                                      views;
    ColormapHelperBase::Ptr                                    colormapHelper;
    PipetteLayerOperations::Ptr                                pipetteLayerOperation;
    Scroom::Utils::StuffList                                   stuff;
    std::map<std::string, Scroom::TiledBitmap::BitmapMetaData> bmd;

  public:
    static TiledBitmapPresentation::Ptr create(std::string                        name_,
                                               Scroom::Utils::Rectangle<int>      rect_,
                                               TiledBitmapInterface::Ptr          tbi_,
                                               std::map<std::string, std::string> properties_,
                                               ColormapHelperBase::Ptr            colormapHelper_,
                                               PipetteLayerOperations::Ptr        pipetteLayerOperation_)
    {
      return Ptr(new TiledBitmapPresentation(std::move(name_),
                                             std::move(rect_),
                                             std::move(tbi_),
                                             std::move(properties_),
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
    // imageMdInterface
    ////////////////////////////////////////////////////////////////////////

    void showMetadata() override;
    void getMap(std::map<std::string, Scroom::TiledBitmap::BitmapMetaData> bm) { bmd = bm; }

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

    ////////////////////////////////////////////////////////////////////////
    // Helpers
    ////////////////////////////////////////////////////////////////////////

    void add(Scroom::Utils::Stuff s) { stuff.push_back(std::move(s)); }

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
                            ColormapHelperBase::Ptr&&            colormapHelper_,
                            PipetteLayerOperations::Ptr&&        pipetteLayerOperation_)
      : name(name_)
      , rect(rect_)
      , tbi(tbi_)
      , properties(properties_)
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
  // imageMdInterface
  ////////////////////////////////////////////////////////////////////////

  /**
   * Show all metadata in the image properties window for a tiledbitmappresentation
   * @override the base showMetadata() function in PresentationBase
   */
  void TiledBitmapPresentation::showMetadata()
  {

    GtkSizeGroup *group, *group2, *group3, *group4, *group5, *group6;
    GtkWidget *window, *grid, *label, *label2, *label3, *label4, *label5, *label6, *label7, *label8, *label9, *label10, *label11,
      *label12;

    // Check for which file is the metadata requested
    auto        it       = bmd.find(getTitle());
    std::string fileName = "Properties: " + getTitle().substr(getTitle().find_last_of("/\\") + 1);

    // Store values for properties in the correct type for the gtk label
    std::string aspect_ratio = "1.00 : 1.00";
    if(it->second.aspectRatio)
    {
      float             aspect_x = it->second.aspectRatio->x;
      float             aspect_y = it->second.aspectRatio->y;
      std::string       sign     = ":";
      std::stringstream stream;
      std::stringstream stream2;
      stream << std::fixed << std::setprecision(2) << aspect_x;
      stream2 << std::fixed << std::setprecision(2) << aspect_y;
      std::string str_aspect_x = stream.str();
      std::string str_aspect_y = stream2.str();
      aspect_ratio             = str_aspect_x + " " + sign + " " + str_aspect_y;
    }

    // Create properties window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title((GtkWindow*)window, fileName.c_str());
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);


    // Add properties value to the labels
    label = gtk_label_new("Color representation: ");
    gtk_widget_modify_font(label, pango_font_description_from_string("Sans Bold 10"));
    gtk_grid_attach(GTK_GRID(grid), label, NULL, GTK_POS_RIGHT, 3, 3);

    label2 = gtk_label_new(it->second.type.c_str());
    gtk_widget_modify_font(label2, pango_font_description_from_string("Sans 10"));
    gtk_grid_attach_next_to(GTK_GRID(grid), label2, label, GTK_POS_RIGHT, 3, 3);

    label3 = gtk_label_new("Sample per pixels: ");
    gtk_widget_modify_font(label3, pango_font_description_from_string("Sans Bold 10"));

    gtk_grid_attach_next_to(GTK_GRID(grid), label3, label, GTK_POS_BOTTOM, 3, 3);

    label4 = gtk_label_new(std::to_string(it->second.samplesPerPixel).c_str());
    gtk_widget_modify_font(label4, pango_font_description_from_string("Sans 10"));
    gtk_grid_attach_next_to(GTK_GRID(grid), label4, label3, GTK_POS_RIGHT, 3, 3);

    label5 = gtk_label_new("Bits per sample: ");
    gtk_widget_modify_font(label5, pango_font_description_from_string("Sans Bold 10"));
    gtk_grid_attach_next_to(GTK_GRID(grid), label5, label3, GTK_POS_BOTTOM, 3, 3);

    label6 = gtk_label_new(std::to_string(it->second.bitsPerSample).c_str());
    gtk_widget_modify_font(label6, pango_font_description_from_string("Sans 10"));
    gtk_grid_attach_next_to(GTK_GRID(grid), label6, label5, GTK_POS_RIGHT, 3, 3);

    label7 = gtk_label_new("Aspect ratio: ");
    gtk_widget_modify_font(label7, pango_font_description_from_string("Sans Bold 10"));
    gtk_grid_attach_next_to(GTK_GRID(grid), label7, label5, GTK_POS_BOTTOM, 3, 3);

    label8 = gtk_label_new(aspect_ratio.c_str());
    gtk_widget_modify_font(label8, pango_font_description_from_string("Sans 10"));
    gtk_grid_attach_next_to(GTK_GRID(grid), label8, label7, GTK_POS_RIGHT, 3, 3);

    label9 = gtk_label_new("Width: ");
    gtk_widget_modify_font(label9, pango_font_description_from_string("Sans Bold 10"));
    gtk_grid_attach_next_to(GTK_GRID(grid), label9, label7, GTK_POS_BOTTOM, 3, 3);

    label10 = gtk_label_new(std::to_string(it->second.rect.getWidth()).c_str());
    gtk_widget_modify_font(label8, pango_font_description_from_string("Sans 10"));
    gtk_grid_attach_next_to(GTK_GRID(grid), label10, label9, GTK_POS_RIGHT, 3, 3);

    label11 = gtk_label_new("Height: ");
    gtk_widget_modify_font(label11, pango_font_description_from_string("Sans Bold 10"));
    gtk_grid_attach_next_to(GTK_GRID(grid), label11, label9, GTK_POS_BOTTOM, 3, 3);

    label12 = gtk_label_new(std::to_string(it->second.rect.getHeight()).c_str());
    gtk_widget_modify_font(label12, pango_font_description_from_string("Sans 10"));
    gtk_grid_attach_next_to(GTK_GRID(grid), label12, label11, GTK_POS_RIGHT, 3, 3);

    // Create separate groups for the view
    group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(group, label);
    gtk_size_group_add_widget(group, label2);

    group2 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(group2, label3);
    gtk_size_group_add_widget(group2, label4);

    group3 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(group3, label5);
    gtk_size_group_add_widget(group3, label6);

    group4 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(group4, label7);
    gtk_size_group_add_widget(group4, label8);

    group5 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(group5, label9);
    gtk_size_group_add_widget(group5, label10);

    group6 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(group6, label11);
    gtk_size_group_add_widget(group6, label12);

    // Display the widgets
    gtk_widget_show_all(window);
    gtk_widget_grab_focus(window);
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
    auto t = openTiledBitmapInterface->open(fileName);

    // Map to hold the bitmap medata values for a specific file
    std::map<std::string, Scroom::TiledBitmap::BitmapMetaData> bm;
    bm.insert({fileName, std::move(std::get<0>(t))});
    auto           it          = bm.find(fileName);
    Layer::Ptr     bottomLayer = std::move(std::get<1>(t));
    ReloadFunction load        = std::move(std::get<2>(t));

    auto                    lsr            = LayerSpecForBitmap(it->second);
    LayerSpec               layerSpec      = std::move(std::get<0>(lsr));
    ColormapHelperBase::Ptr colormapHelper = std::move(std::get<1>(lsr));

    PresentationInterface::Ptr result;
    if(bottomLayer && !layerSpec.empty())
    {
      auto                        tiledBitmap           = TiledBitmap::create(bottomLayer, layerSpec);
      PipetteLayerOperations::Ptr pipetteLayerOperation = boost::dynamic_pointer_cast<PipetteLayerOperations>(layerSpec[0]);

      std::map<std::string, std::string> properties;
      if(it->second.colormapHelper)
      {
        properties = it->second.colormapHelper->getProperties();
      }
      if(pipetteLayerOperation)
      {
        properties[PIPETTE_PROPERTY_NAME] = "";
      }

      auto tiledBitmapPresentation = TiledBitmapPresentation::create(
        fileName, it->second.rect, tiledBitmap, properties, colormapHelper, pipetteLayerOperation);
      tiledBitmapPresentation->getMap(bm);
      tiledBitmapPresentation->add(load(tiledBitmap->progressInterface()));

      if(it->second.aspectRatio)
      {
        result = TransformPresentation::create(tiledBitmapPresentation, TransformationData::create(*it->second.aspectRatio));
      }
      else
      {
        result = tiledBitmapPresentation;
      }
    }
    return result;
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
