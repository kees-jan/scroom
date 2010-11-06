#include "tiff.hh"

#include "tiffpresentation.hh"

Tiff::~Tiff()
{
}

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

std::string Tiff::getPluginName()
{
  return "Tiff";
}

std::string Tiff::getPluginVersion()
{
  return "0.0";
}

void Tiff::registerCapabilities(ScroomInterface* host)
{
  host->registerOpenInterface("Tiff viewer", this);
}

void Tiff::unregisterCapabilities(ScroomInterface* host)
{
  host->unregisterOpenInterface(this);
}

////////////////////////////////////////////////////////////////////////
// OpenInterface
////////////////////////////////////////////////////////////////////////

std::list<GtkFileFilter*> Tiff::getFilters()
{
  std::list<GtkFileFilter*> result;

  GtkFileFilter* filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "Tiff files");
  gtk_file_filter_add_mime_type(filter, "image/tiff");
  result.push_back(filter);
  
  return result;
}
  
PresentationInterface::Ptr Tiff::open(const std::string& fileName, FileOperationObserver* observer)
{
  TiffPresentation* p = new TiffPresentation();
  if(!p->load(fileName, observer))
  {
    delete p;
    p=NULL;
  }
  return PresentationInterface::Ptr(p);
}
