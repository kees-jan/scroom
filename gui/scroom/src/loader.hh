#ifndef LOADER_HH
#define LOADER_HH

#include <list>

#include <boost/thread/mutex.hpp>

#include <gtk/gtk.h>

#include <scroominterface.hh>

class Loader : private FileOperationObserver
{
private:
  boost::mutex remainingFilesMutex;
  std::list<GtkFileFilterInfo> remainingFiles;
  bool currentlyLoading;

public:
  static Loader& getInstance();
  
public:
  Loader();
  virtual ~Loader();

  virtual void load(const GtkFileFilterInfo& info);

  // Helpers /////////////////////////////////////////////////////////////
public:
  virtual void next();
  
private:
  virtual void fileOperationComplete();
};

#endif
