#ifndef _COLORMAPPLUGIN_HH
#define _COLORMAPPLUGIN_HH

#include <plugininformationinterface.hh>

/**
 * Register the ColormapPlugin, keep track of PresentationInterface instances
 */
class ColormapPlugin : public PluginInformationInterface, public PresentationObserver
{
private:
  std::list<PresentationInterface::WeakPtr> presentations;
  
public:
  ColormapPlugin();
  virtual ~ColormapPlugin();

public:
  virtual std::string getPluginName();
  virtual std::string getPluginVersion();
  virtual void registerCapabilities(ScroomInterface* host);
  virtual void unregisterCapabilities(ScroomInterface* host);

  virtual void presentationAdded(PresentationInterface::Ptr p);
  virtual void presentationDeleted();

};

#endif
