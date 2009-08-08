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
  // char buffer[] = "Hello world!";
  // 
  // cairo_move_to(cr, 30, 30);
  // cairo_show_text(cr, buffer);

  double pp=1.0;
  if(zoom >=0)
    pp *= 1<<zoom;
  else
    pp /= 1<<(-zoom);

  printf("One presentation pixel amounts to %f screen pixels\n", pp);

  int xorig = (int)(-presentationArea.x*pp);
  int yorig = (int)(-presentationArea.y*pp);

  for(int i=-500; i<500; i+=50)
  {
    cairo_move_to(cr, xorig-500*pp, yorig+i*pp);
    cairo_line_to(cr, xorig+500*pp, yorig+i*pp);
    cairo_move_to(cr, xorig+i*pp, yorig-500*pp);
    cairo_line_to(cr, xorig+i*pp, yorig+500*pp);
  }
  cairo_move_to(cr, xorig-500*pp, yorig-500*pp);
  cairo_line_to(cr, xorig+500*pp, yorig+500*pp);
  cairo_move_to(cr, xorig-500*pp, yorig+500*pp);
  cairo_line_to(cr, xorig+500*pp, yorig-500*pp);

  cairo_stroke(cr);
}
