#ifndef _TIFF_HH
#define _TIFF_HH

#include <plugininformationinterface.hh>
#include <presentationinterface.hh>

class Tiff : public PluginInformationInterface, public OpenInterface, public NewInterface
{
public:

  ////////////////////////////////////////////////////////////////////////
  // PluginInformationInterface
  
  virtual std::string getPluginName();
  virtual std::string getPluginVersion();
  virtual void registerCapabilities(ScroomInterface* host);
  virtual void unregisterCapabilities(ScroomInterface* host);

  ////////////////////////////////////////////////////////////////////////
  // OpenInterface
  
  virtual std::list<GtkFileFilter*> getFilters();
  virtual PresentationInterface* open(const std::string& fileName, FileOperationObserver* observer);

  ////////////////////////////////////////////////////////////////////////
  // NewInterface (for development only)
  
  virtual PresentationInterface* createNew(FileOperationObserver* observer);

  ////////////////////////////////////////////////////////////////////////
  
  virtual ~Tiff();
};

#endif
