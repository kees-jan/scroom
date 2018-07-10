#include <scroom/transformpresentation.hh>

#include <scroom/rectangle.hh>

TransformationData::TransformationData()
  : aspectRatio(1,1)
{}

TransformationData::Ptr TransformationData::create()
{
  return Ptr(new TransformationData());
}

void TransformationData::setAspectRatio(double x, double y)
{
  aspectRatio = make_point(x,y);
}

Point<double> TransformationData::getAspectRatio() const
{
  return aspectRatio;
}

////////////////////////////////////////////////////////////////////////

TransformPresentation::TransformPresentation(PresentationInterface::Ptr const& presentation, TransformationData::Ptr const& transformationData)
  : transformationData(transformationData), presentation(presentation),
    colormappable(boost::dynamic_pointer_cast<Colormappable>(presentation))
{}

TransformPresentation::Ptr TransformPresentation::create(PresentationInterface::Ptr const& presentation, TransformationData::Ptr const& transformationData)
{
  return Ptr(new TransformPresentation(presentation, transformationData));
}

Rectangle<double> TransformPresentation::getRect()
{
  return presentation->getRect() * transformationData->getAspectRatio();
}

void TransformPresentation::open(ViewInterface::WeakPtr viewInterface)
{
  presentation->open(viewInterface);
}

void TransformPresentation::close(ViewInterface::WeakPtr viewInterface)
{
  presentation->close(viewInterface);
}

void TransformPresentation::redraw(ViewInterface::Ptr const& vi, cairo_t* cr,
                                   Rectangle<double> presentationArea, int zoom)
{
  presentation->redraw(vi, cr, presentationArea, zoom);
}

bool TransformPresentation::getProperty(const std::string& name, std::string& value)
{
  return presentation->getProperty(name, value);
}

bool TransformPresentation::isPropertyDefined(const std::string& name)
{
  return presentation->isPropertyDefined(name);
}

std::string TransformPresentation::getTitle()
{
  return presentation->getTitle();
}

void TransformPresentation::setColormap(Colormap::Ptr colormap)
{
  colormappable->setColormap(colormap);
}

Colormap::Ptr TransformPresentation::getOriginalColormap()
{
  return colormappable->getOriginalColormap();
}

int TransformPresentation::getNumberOfColors()
{
  return colormappable->getNumberOfColors();
}

Color TransformPresentation::getMonochromeColor()
{
  return colormappable->getMonochromeColor();
}

void TransformPresentation::setMonochromeColor(const Color& c)
{
  colormappable->setMonochromeColor(c);
}

void TransformPresentation::setTransparentBackground()
{
  colormappable->setTransparentBackground();
}

void TransformPresentation::disableTransparentBackground()
{
  colormappable->disableTransparentBackground();
}

bool TransformPresentation::getTransparentBackground()
{
  return colormappable->getTransparentBackground();
}
