#include "loader.hh"

#include <stdio.h>

#include <threadpool.hh>

#include "callbacks.hh"
#include "pluginmanager.hh"


////////////////////////////////////////////////////////////////////////

class FileOperation : public SeqJob, public FileOperationObserver
{
public:
  virtual ~FileOperation() {}

  virtual void fileOperationComplete()
  {
    done();
  }
};

////////////////////////////////////////////////////////////////////////

class LoadOperation : public FileOperation
{
private:
  GtkFileFilterInfo info;

public:
  LoadOperation(const GtkFileFilterInfo& info);
  virtual ~LoadOperation() {}

  virtual bool doWork();
};

LoadOperation::LoadOperation(const GtkFileFilterInfo& info)
  : info(info)
{
}

bool LoadOperation::doWork()
{
  const std::map<OpenInterface*, std::string>& openInterfaces = PluginManager::getInstance().getOpenInterfaces();
  PresentationInterface* presentation = NULL;
  for(std::map<OpenInterface*, std::string>::const_iterator cur=openInterfaces.begin();
      cur != openInterfaces.end() && presentation==NULL;
      cur++)
  {
    std::list<GtkFileFilter*> filters = cur->first->getFilters();
    for(std::list<GtkFileFilter*>::iterator f = filters.begin();
        f != filters.end() && presentation==NULL;
        f++)
    {
      if(gtk_file_filter_filter(*f, &info))
      {
        presentation = cur->first->open(info.filename, this);
        find_or_create_scroom(presentation);
        return FALSE;
      }
    }
    // Failed??? Try next one...
    fileOperationComplete();
  }

  return FALSE;
}
  

////////////////////////////////////////////////////////////////////////

class CreateOperation : public FileOperation
{
private:
  NewInterface* interface;

public:
  CreateOperation(NewInterface* interface);
  virtual ~CreateOperation() {}

  virtual bool doWork();
};

CreateOperation::CreateOperation(NewInterface* interface)
  : interface(interface)
{
}

bool CreateOperation::doWork()
{
  PresentationInterface* presentation = interface->createNew(this);
  find_or_create_scroom(presentation);
  return FALSE;
}

////////////////////////////////////////////////////////////////////////

void create(NewInterface* interface)
{
  sequentially(new CreateOperation(interface));
}

void load(const GtkFileFilterInfo& info)
{
  sequentially(new LoadOperation(info));
}


