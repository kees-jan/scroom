#ifndef PROGRESSBARMANAGER_HH
#define PROGRESSBARMANAGER_HH

#include <gtk/gtk.h>

#include <scroom/viewinterface.hh>

class ProgressBarManager : public ProgressInterface
{
private:
  GtkProgressBar* progressBar;
  State state;
  
public:
  ProgressBarManager(GtkProgressBar* progressBar=NULL);
  ~ProgressBarManager();

  void setProgressBar(GtkProgressBar* progressBar);
  
  // ProgressInterface ///////////////////////////////////////////////////
  
  virtual void setState(State s);
  virtual void setProgress(double d);
  virtual void setProgress(int done, int total);
};

#endif
