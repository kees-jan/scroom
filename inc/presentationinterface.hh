#ifndef _PRESENTATIONINTERFACE_H
#define _PRESENTATIONINTERFACE_H

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <gdk/gdk.h>
#include <cairo.h>

#include <viewinterface.hh>

class ViewIdentifier
{
public:
  virtual ~ViewIdentifier()
  {}
};

class PresentationInterface
{
public:
  typedef boost::shared_ptr<PresentationInterface> Ptr;
  typedef boost::weak_ptr<PresentationInterface> WeakPtr;
 
  virtual ~PresentationInterface()
  {
  }

  virtual GdkRectangle getRect()=0;

  virtual ViewIdentifier* open(ViewInterface* viewInterface)=0;
  virtual void redraw(ViewIdentifier* vid, cairo_t* cr, GdkRectangle presentationArea, int zoom)=0;
  virtual void close(ViewIdentifier* vid)=0;
  virtual bool getProperty(const std::string& name, std::string& value)=0;
  virtual bool isPropertyDefined(const std::string& name)=0;
  
};

#endif
