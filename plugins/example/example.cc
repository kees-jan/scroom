#include "example.hh"

#include <gdk/gdk.h>

#include "examplepresentation.hh"

////////////////////////////////////////////////////////////////////////

gboolean reportComplete(gpointer data)
{
  if(data)
  {
    ((FileOperationObserver*)data)->fileOperationComplete();
  }

  return FALSE;
}

////////////////////////////////////////////////////////////////////////

Example::~Example()
{
}

std::string Example::getPluginName()
{
  return "Example";
}

std::string Example::getPluginVersion()
{
  return "0.0";
}

void Example::registerCapabilities(ScroomInterface* host)
{
  host->registerNewInterface("Example", this);
}

void Example::unregisterCapabilities(ScroomInterface* host)
{
  host->unregisterNewInterface(this);
}

PresentationInterface::Ptr Example::createNew(FileOperationObserver* observer)
{
  if(observer)
  {
    gdk_threads_add_idle(reportComplete, observer);
  }
  return PresentationInterface::Ptr(new ExamplePresentation());
}
  
