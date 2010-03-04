#ifndef COLORMAPPABLE_HH
#define COLORMAPPABLE_HH

#include <boost/shared_ptr.hpp>

#include <presentationinterface.hh>
#include <observable.hh>

#define COLORMAPPABLE_PROPERTY_NAME "Colormappable"

class Colormap
{
public:
  typedef boost::shared_ptr<Colormap> Ptr;
  typedef boost::weak_ptr<Colormap> WeakPtr;
};

class Colormappable: public Observable<Viewable>
{
  virtual void setColormap(Colormap::Ptr colormap)=0;
};

#endif
