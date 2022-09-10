#include <string>
#include <utility>

#include <cairo.h>

#include <scroom/color.hh>
#include <scroom/colormappable.hh>
#include <scroom/pipettelayeroperations.hh>
#include <scroom/pipetteviewinterface.hh>
#include <scroom/point.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/rectangle.hh>
#include <scroom/transformpresentation.hh>
#include <scroom/viewinterface.hh>

#include "scroom/cairo-helpers.hh"

TransformationData::TransformationData()
  : aspectRatio(1, 1)
{
}

TransformationData::TransformationData(Scroom::Utils::Point<double> aspectRatio_)
  : aspectRatio(aspectRatio_)
{
}

TransformationData::Ptr TransformationData::create() { return Ptr(new TransformationData()); }

TransformationData::Ptr TransformationData::create(Scroom::Utils::Point<double> aspectRatio_)
{
  return Ptr(new TransformationData(aspectRatio_));
}

void TransformationData::setAspectRatio(double x, double y) { aspectRatio = Scroom::Utils::make_point(x, y); }
void TransformationData::setAspectRatio(Scroom::Utils::Point<double> aspectRatio_) { aspectRatio = aspectRatio_; }

Scroom::Utils::Point<double> TransformationData::getAspectRatio() const { return aspectRatio; }

////////////////////////////////////////////////////////////////////////

TransformPresentation::TransformPresentation(PresentationInterface::Ptr const& presentation_,
                                             TransformationData::Ptr           transformationData_)
  : transformationData(std::move(transformationData_))
  , presentation(presentation_)
  , colormappable(std::dynamic_pointer_cast<Colormappable>(presentation_))
  , showMetaDataInterface(std::dynamic_pointer_cast<ShowMetadataInterface>(presentation_))
{
}

TransformPresentation::Ptr TransformPresentation::create(PresentationInterface::Ptr const& presentation,
                                                         TransformationData::Ptr const&    transformationData)
{
  return Ptr(new TransformPresentation(presentation, transformationData));
}

Scroom::Utils::Rectangle<double> TransformPresentation::getRect()
{
  return presentation->getRect() * transformationData->getAspectRatio();
}

void TransformPresentation::open(ViewInterface::WeakPtr viewInterface)
{
  auto [it, inserted] = viewData.insert({viewInterface, Detail::ViewData::create(viewInterface)});
  require(inserted);
  auto& [_, data] = *it;

  presentation->open(data);
  PresentationBase::open(viewInterface);
}

void TransformPresentation::close(ViewInterface::WeakPtr viewInterface)
{
  auto data = viewData.at(viewInterface);
  viewData.erase(viewInterface);

  PresentationBase::close(viewInterface);
  presentation->close(data);
}

void TransformPresentation::redraw(ViewInterface::Ptr const&        vi,
                                   cairo_t*                         cr,
                                   Scroom::Utils::Rectangle<double> presentationArea,
                                   int                              zoom)
{
  Scroom::Utils::Point<double> const aspectRatio = transformationData->getAspectRatio();
  const auto                         data        = viewData.at(vi);

  if(data->presentationArea != presentationArea || data->zoom != zoom)
  {
    data->image.reset();
  }
  if(!data->image)
  {
    auto pixelSize = pixelSizeFromZoom(zoom);
    auto viewArea  = ceil((presentationArea * pixelSize / aspectRatio).getSize()).to<int>();

    data->image = Scroom::Bitmap::BitmapSurface::create(viewArea.x, viewArea.y, CAIRO_FORMAT_ARGB32);

    cairo_surface_t* surface  = data->image->get();
    cairo_t*         image_cr = cairo_create(surface);

    presentation->redraw(data, image_cr, presentationArea / aspectRatio, zoom);

    cairo_destroy(image_cr);
  }

  cairo_save(cr);
  cairo_scale(cr, aspectRatio.x, aspectRatio.y);
  cairo_set_source_surface(cr, data->image->get(), 0, 0);
  cairo_paint(cr);
  cairo_restore(cr);
}

bool TransformPresentation::getProperty(const std::string& name, std::string& value)
{
  return presentation->getProperty(name, value);
}

bool TransformPresentation::isPropertyDefined(const std::string& name) { return presentation->isPropertyDefined(name); }

std::string TransformPresentation::getTitle() { return presentation->getTitle(); }

void TransformPresentation::setColormap(Colormap::Ptr colormap) { colormappable->setColormap(colormap); }

Colormap::Ptr TransformPresentation::getOriginalColormap() { return colormappable->getOriginalColormap(); }

int TransformPresentation::getNumberOfColors() { return colormappable->getNumberOfColors(); }

Color TransformPresentation::getMonochromeColor() { return colormappable->getMonochromeColor(); }

void TransformPresentation::setMonochromeColor(const Color& c) { colormappable->setMonochromeColor(c); }

void TransformPresentation::setTransparentBackground() { colormappable->setTransparentBackground(); }

void TransformPresentation::disableTransparentBackground() { colormappable->disableTransparentBackground(); }

bool TransformPresentation::getTransparentBackground() { return colormappable->getTransparentBackground(); }

void TransformPresentation::showMetadata(GtkWindow* parent) { showMetaDataInterface->showMetadata(parent); }

PipetteLayerOperations::PipetteColor TransformPresentation::getPixelAverages(Scroom::Utils::Rectangle<double> area)
{
  PipetteViewInterface::Ptr const pipettePresentation = std::dynamic_pointer_cast<PipetteViewInterface>(presentation);
  require(pipettePresentation);
  return pipettePresentation->getPixelAverages(area / getAspectRatio());
}

Scroom::Utils::Point<double> TransformPresentation::getAspectRatio() const { return transformationData->getAspectRatio(); }

namespace Detail
{
  ViewData::ViewData(ViewInterface::WeakPtr parent_)
    : weakParent(std::move(parent_))
  {
  }

  ViewInterface::Ptr ViewData::parent() const { return ViewInterface::Ptr(weakParent); }

  void ViewData::invalidate()
  {
    image.reset();
    parent()->invalidate();
  }

  ProgressInterface::Ptr ViewData::getProgressInterface() { return parent()->getProgressInterface(); }
  void                   ViewData::addSideWidget(std::string title, GtkWidget* w) { parent()->addSideWidget(title, w); }
  void                   ViewData::removeSideWidget(GtkWidget* w) { parent()->removeSideWidget(w); }
  void                   ViewData::addToToolbar(GtkToolItem* ti) { parent()->addToToolbar(ti); }
  void                   ViewData::removeFromToolbar(GtkToolItem* ti) { parent()->removeFromToolbar(ti); }
  void ViewData::registerSelectionListener(SelectionListener::Ptr ptr) { parent()->registerSelectionListener(ptr); }
  void ViewData::registerPostRenderer(PostRenderer::Ptr ptr) { parent()->registerPostRenderer(ptr); }
  void ViewData::setStatusMessage(const std::string& string) { parent()->setStatusMessage(string); }
  std::shared_ptr<PresentationInterface> ViewData::getCurrentPresentation() { return parent()->getCurrentPresentation(); }
  void ViewData::addToolButton(GtkToggleButton* button, ToolStateListener::Ptr ptr) { parent()->addToolButton(button, ptr); }
  ViewData::Ptr ViewData::create(const ViewInterface::WeakPtr& parent) { return ViewData::Ptr(new ViewData(parent)); }
} // namespace Detail