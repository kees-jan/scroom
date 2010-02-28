#ifndef COLORMAPPROVIDER_HH
#define COLORMAPPROVIDER_HH

#include <list>

#include <presentationinterface.hh>

class ColormapProvider: public Viewable
{
private:
  PresentationInterface::WeakPtr presentation;
  std::list<ViewInterface*> views;
  
public:
  ColormapProvider(PresentationInterface::Ptr p);
  ~ColormapProvider();

  virtual void open(ViewInterface* vi);
  virtual void close(ViewInterface* vi);
  
};

#endif
