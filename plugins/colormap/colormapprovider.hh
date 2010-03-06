#ifndef COLORMAPPROVIDER_HH
#define COLORMAPPROVIDER_HH

#include <map>

#include <gtk/gtk.h>

#include <presentationinterface.hh>

class ColormapProvider: public Viewable
{
private:
  PresentationInterface::WeakPtr presentation;
  std::map<ViewInterface*, GtkTreeView*> views;
  
public:
  ColormapProvider(PresentationInterface::Ptr p);
  ~ColormapProvider();

  
  // Viewable ////////////////////////////////////////////////////////////  
  virtual void open(ViewInterface* vi);
  virtual void close(ViewInterface* vi);

  // Helpers /////////////////////////////////////////////////////////////
  void on_colormap_selected(GtkTreeView* tv);
  
};

#endif
