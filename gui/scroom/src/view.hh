#ifndef _VIEW_HH
#define _VIEW_HH

#include <glade/glade.h>

#include <presentationinterface.hh>

class View
{
public:

  View(GladeXML* scroomXml, PresentationInterface* presentation);
};

#endif
