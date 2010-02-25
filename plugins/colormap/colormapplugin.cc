#include "colormapplugin.hh"

// #include <gdk/gdk.h>

// #include "examplepresentation.hh"

////////////////////////////////////////////////////////////////////////

ColormapPlugin::ColormapPlugin()
  : presentations()
{
}

ColormapPlugin::~ColormapPlugin()
{
}

std::string ColormapPlugin::getPluginName()
{
  return "Colormap";
}

std::string ColormapPlugin::getPluginVersion()
{
  return "0.0";
}

void ColormapPlugin::registerCapabilities(ScroomInterface* host)
{
  host->registerPresentationObserver("Colormap", this);
}

void ColormapPlugin::unregisterCapabilities(ScroomInterface* host)
{
  host->unregisterPresentationObserver(this);
}

void ColormapPlugin::presentationAdded(PresentationInterface::Ptr p)
{
  printf("ColormapPlugin: A presentation was created\n");
}

void ColormapPlugin::presentationDeleted()
{
  printf("ColormapPlugin: A presentation may have been deleted\n");
}
  
