#ifndef _VIEW_HH
#define _VIEW_HH

#include <glade/glade.h>
#include <cairo.h>

#include <presentationinterface.hh>

class View
{
private:
  GladeXML* scroomXml;
  PresentationInterface* presentation;
  
public:

  View(GladeXML* scroomXml, PresentationInterface* presentation);

  void redraw(cairo_t* cr);
  bool hasPresentation();
  void setPresentation(PresentationInterface* presentation);
};

#endif
