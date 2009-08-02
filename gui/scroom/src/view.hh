#ifndef _VIEW_HH
#define _VIEW_HH

#include <map>

#include <glade/glade.h>
#include <cairo.h>

#include <scroominterface.hh>

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


  ////////////////////////////////////////////////////////////////////////
  // Scroom events
  
  void on_newInterfaces_update(const std::map<NewInterface*, std::string>& newInterfaces);


  ////////////////////////////////////////////////////////////////////////
  // Presentation events
};

#endif
