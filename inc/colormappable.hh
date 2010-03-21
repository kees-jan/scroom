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
  Color()
    : red(0.0), green(0.0), blue(0.0)
  {}
  
  Color(double red, double green, double blue)
    : red(red), green(green), blue(blue)
  {}
  
  Color(double gray)
    : red(gray), green(gray), blue(gray)
  {}
};

class Colormap
{
public:
  typedef boost::shared_ptr<Colormap> Ptr;
  typedef boost::weak_ptr<Colormap> WeakPtr;

public:
  std::vector<Color> colors;

public:
  static Colormap::Ptr create()
  {
    return Colormap::Ptr(new Colormap());
  }

  static Colormap::Ptr createDefault(int n)
  {
    Colormap::Ptr result=create();
    result->colors.reserve(n);
    result->colors.clear();
    double max = n-1;
    for(int i=0; i<n; i++)
      result->colors.push_back(Color((max-i)/max));  // Min is white

    return result;
  }
};

class Colormappable: public Observable<Viewable>
{
public:
  ~Colormappable() {}
  
  virtual void setColormap(Colormap::Ptr colormap)=0;
};

#endif
