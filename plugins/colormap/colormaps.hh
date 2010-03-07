#ifndef COLORMAPS_HH
#define COLORMAPS_HH

#include <gtk/gtk.h>

#include<colormappable.hh>

class Colormaps
{
private:
  GtkListStore* filenames;

private:
  Colormaps();
  ~Colormaps();

public:
  static Colormaps& getInstance();
  
  GtkListStore* getFileNames();

  void select(GtkTreeIter iter, Colormappable* colormappable);
  Colormap::Ptr load(const char* name);
};

enum
  {
    COLUMN_NAME,
    COLUMN_POINTER,
    COLUMN_ENABLED,
    N_COLUMNS
  };

#endif
