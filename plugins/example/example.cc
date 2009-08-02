#include "example.hh"

#include "examplepresentation.hh"

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

PresentationInterface* Example::createNew()
{
  return new ExamplePresentation();
}
  
