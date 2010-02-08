#ifndef _SCROOMINTERFACE_HH
#define _SCROOMINTERFACE_HH

#include <string>
#include <list>

#include <gtk/gtk.h>

#include <presentationinterface.hh>

class FileOperationObserver
{
public:
  virtual ~FileOperationObserver() {}

  virtual void fileOperationComplete()=0;
};

class NewInterface
{
public:
  virtual ~NewInterface() {}
  
  virtual PresentationInterface::Ptr createNew(FileOperationObserver* observer)=0;
};

class OpenInterface
{
public:
  virtual ~OpenInterface() {}

  virtual std::list<GtkFileFilter*> getFilters()=0;
  
  virtual PresentationInterface::Ptr open(const std::string& fileName, FileOperationObserver* observer)=0;
};

class PresentationObserver
{
public:
  virtual ~PresentationObserver() {}

  virtual void presentationAdded()=0;
  virtual void persentationDeleted()=0;
};

class ViewObserver
{
public:
  virtual ~ViewObserver() {}

  virtual void viewAdded()=0;
  virtual void viewDeleted()=0;
};

class ScroomInterface
{
public:
  virtual ~ScroomInterface() {}

  virtual void registerNewInterface(const std::string& identifier, NewInterface* newInterface)=0;
  virtual void unregisterNewInterface(NewInterface* newInterface)=0;

  virtual void registerOpenInterface(const std::string& identifier, OpenInterface* openInterface)=0;
  virtual void unregisterOpenInterface(OpenInterface* openInterface)=0;

  virtual void registerViewObserver(const std::string& identifier, ViewObserver* observer)=0;
  virtual void unregisterViewObserver(ViewObserver* observer)=0;

  virtual void registerPresentationObserver(const std::string& identifier, PresentationObserver* observer)=0;
  virtual void unregisterPresentationObserver(PresentationObserver* observer)=0;
};


#endif
