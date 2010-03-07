#ifndef COLORMAPPABLE_HH
#define COLORMAPPABLE_HH

#include <boost/shared_ptr.hpp>

#include <presentationinterface.hh>
#include <observable.hh>

#define COLORMAPPABLE_PROPERTY_NAME "Colormappable"

class Color
{
public:
  double red;
  double green;
  double blue;

public:
  Color();
  Color(double red, double green, double blue);
};

class Colormap
{
public:
  typedef boost::shared_ptr<Colormap> Ptr;
  typedef boost::weak_ptr<Colormap> WeakPtr;

public:
  std::vector<Color> colors;

public:
  static Colormap::Ptr create();
};

class Colormappable: public Observable<Viewable>
{
public:
  ~Colormappable() {}
  
  virtual void setColormap(Colormap::Ptr colormap)=0;
};

#endif
