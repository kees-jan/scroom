#include "tiff.hh"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

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
  host->registerNewInterface("Tiff", this);
  host->registerOpenInterface("Tiff viewer", this);
}

void Tiff::unregisterCapabilities(ScroomInterface* host)
{
  host->unregisterNewInterface(this);
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
#if MUTRACX_HACKS
  gtk_file_filter_add_pattern(filter, "*.tif");
  gtk_file_filter_add_pattern(filter, "*.tiff");
  gtk_file_filter_add_pattern(filter, "*.TIF");
  gtk_file_filter_add_pattern(filter, "*.TIFF");
#else
  gtk_file_filter_add_mime_type(filter, "image/tiff");
#endif
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

////////////////////////////////////////////////////////////////////////
// NewInterface
////////////////////////////////////////////////////////////////////////

PresentationInterface::Ptr Tiff::createNew(FileOperationObserver* observer)
{
  TiffPresentation* p = new TiffPresentation();
  if(!p->load("tissuebox.tif", observer))
  {
    delete p;
    p=NULL;
  }
  return PresentationInterface::Ptr(p);
}
  
