#ifndef _EXAMPLEPRESENTATION_HH
#define _EXAMPLEPRESENTATION_HH

#include <presentationinterface.hh>

class ExamplePresentation : public PresentationInterface
{
public:
  virtual ~ExamplePresentation();

  virtual GdkRectangle getRect();
  virtual void open(ViewInterface* viewInterface);
  virtual void redraw(ViewInterface* vi, cairo_t* cr, GdkRectangle presentationArea, int zoom);
  virtual void close(ViewInterface* vi);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();
};

#endif
