#include "examplepresentation.hh"

#include <unused.h>

ExamplePresentation::~ExamplePresentation()
{
}

GdkRectangle ExamplePresentation::getRect()
{
  GdkRectangle rect;
  rect.x=-500;
  rect.y=-500;
  rect.width=1000;
  rect.height=1000;

  return rect;
}

void ExamplePresentation::redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  char buffer[] = "Hello world!";
  UNUSED(presentationArea);
  UNUSED(zoom);

  cairo_move_to(cr, 30, 30);
  cairo_show_text(cr, buffer);
}
