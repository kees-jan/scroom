#include "examplepresentation.hh"

ExamplePresentation::~ExamplePresentation()
{
}

int ExamplePresentation::getHeight()
{
  return 1000;
}

int ExamplePresentation::getWidth()
{
  return 1000;
}

void ExamplePresentation::redraw(cairo_t* cr, GdkRectangle presentationArea, int zoomIn, int zoomOut)
{
  char buffer[] = "Hello world!";

  cairo_move_to(cr, 30, 30);
  cairo_show_text(cr, buffer);
}
