#ifndef COLORMAPS_HH
#define COLORMAPS_HH

#include <list>

#include <gtk/gtk.h>

#include<colormappable.hh>


/**
 * Manage the list of colormap files.
 *
 * Upon request, load the selected file, producing a Colormap object.
 *
 * This is a singleton object.
 */
class Colormaps
{
private:
  std::list<Colormap::ConstPtr> colormaps;

private:
  /** Constructor */
  Colormaps();

  /** Destructor */
  ~Colormaps();

public:
  /** Get a reference to the instance */
  static Colormaps& getInstance();

  /**
   * Get a copy of the list of Colormap objects.
   */
  std::list<Colormap::ConstPtr> getColormaps();

  /**
   * Load a colormap by name
   */
  Colormap::Ptr load(const char* name);
};

#endif
