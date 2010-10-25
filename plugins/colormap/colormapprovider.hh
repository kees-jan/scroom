#ifndef COLORMAPPROVIDER_HH
#define COLORMAPPROVIDER_HH

#include <map>

#include <gtk/gtk.h>

#include <presentationinterface.hh>

/**
 * Provide the colormap widget to the Viewable.
 *
 * When the user selects one of the colormaps, they will be set.
 */
class ColormapProvider: public Viewable
{
private:
  /** The presentation to which we're associated */
  PresentationInterface::WeakPtr presentation;

  /** The views to which we're associated */
  std::map<ViewInterface*, GtkTreeView*> views;

  /** The colormaps we're offering to our views */
  GtkListStore* colormaps;
  
public:
  /** Constructor */
  ColormapProvider(PresentationInterface::Ptr p);

  /** Destructor */
  ~ColormapProvider();
  
  // Viewable ////////////////////////////////////////////////////////////

  /** A new view was opened */
  virtual void open(ViewInterface* vi);

  /** An existing view was closed */
  virtual void close(ViewInterface* vi);

  // Helpers /////////////////////////////////////////////////////////////

  /** The user selected a colormap */
  void on_colormap_selected(GtkTreeView* tv);
  
};

#endif
