#ifndef _SCROOMINTERFACE_HH
#define _SCROOMINTERFACE_HH

#include <string>
#include <list>

#include <gtk/gtk.h>

#include <presentationinterface.hh>

class NewInterface
{
public:
  virtual ~NewInterface()
  {
  }
  
  virtual PresentationInterface* createNew()=0;
};

class OpenInterface
{
public:
  virtual ~OpenInterface()
  {
  }

  virtual std::list<GtkFileFilter*> getFilters()=0;
  
  virtual PresentationInterface* open(const std::string& fileName)=0;
};

class ScroomInterface
{
public:
  virtual ~ScroomInterface()
  {
  }

  virtual void registerNewInterface(const std::string& identifier, NewInterface* newInterface)=0;
  virtual void unregisterNewInterface(NewInterface* newInterface)=0;

  virtual void registerOpenInterface(const std::string& identifier, OpenInterface* openInterface)=0;
  virtual void unregisterOpenInterface(OpenInterface* openInterface)=0;
};


#endif
