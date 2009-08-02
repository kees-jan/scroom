#include "view.hh"

View::View(GladeXML* scroomXml, PresentationInterface* presentation)
  : scroomXml(scroomXml), presentation(presentation)
{
}

void View::redraw(cairo_t* cr)
{
  char buffer[] = "View says \"Hi\"";

  cairo_move_to(cr, 50, 50);
  cairo_show_text(cr, buffer);

  if(presentation)
  {
    GdkRectangle rect;
    presentation->redraw(cr, rect, 1,1);
  }
}

bool View::hasPresentation()
{
  return presentation!=NULL;
}

void View::setPresentation(PresentationInterface* presentation)
{
  if(this->presentation)
  {
    delete this->presentation;
    this->presentation=NULL;
  }

  this->presentation = presentation;
}
