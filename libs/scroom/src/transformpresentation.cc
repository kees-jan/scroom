#include <string>

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

TransformationData::TransformationData()
  : aspectRatio(1, 1)
{}

TransformationData::TransformationData(Scroom::Utils::Point<double> aspectRatio_)
  : aspectRatio(aspectRatio_)
{}

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
                                             TransformationData::Ptr const&    transformationData_)
  : transformationData(transformationData_)
  , presentation(presentation_)
  , colormappable(boost::dynamic_pointer_cast<Colormappable>(presentation_))
{}

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
  presentation->open(viewInterface);
  PresentationBase::open(viewInterface);
}

void TransformPresentation::close(ViewInterface::WeakPtr viewInterface)
{
  PresentationBase::close(viewInterface);
  presentation->close(viewInterface);
}

void TransformPresentation::redraw(ViewInterface::Ptr const&        vi,
                                   cairo_t*                         cr,
                                   Scroom::Utils::Rectangle<double> presentationArea,
                                   int                              zoom)
{
  Scroom::Utils::Point<double> aspectRatio = transformationData->getAspectRatio();

  cairo_save(cr);
  cairo_scale(cr, aspectRatio.x, aspectRatio.y);
  presentation->redraw(vi, cr, presentationArea, zoom);
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

void TransformPresentation::showMetadata() { presentation->showMetadata(); }

PipetteLayerOperations::PipetteColor TransformPresentation::getPixelAverages(Scroom::Utils::Rectangle<int> area)
{
  PipetteViewInterface::Ptr pipettePresentation = boost::dynamic_pointer_cast<PipetteViewInterface>(presentation);
  require(pipettePresentation);
  return pipettePresentation->getPixelAverages(area);
}

Scroom::Utils::Point<double> TransformPresentation::getAspectRatio() const { return transformationData->getAspectRatio(); }
