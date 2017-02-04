/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef PROGRESSBARMANAGER_HH
#define PROGRESSBARMANAGER_HH

#include <gtk/gtk.h>

#include <scroom/viewinterface.hh>

class ProgressBarManager : public ProgressInterface
{
public:
  typedef boost::shared_ptr<ProgressBarManager> Ptr;
  
private:
  GtkProgressBar* progressBar;
  bool isWaiting;

private:
  ProgressBarManager(GtkProgressBar* progressBar);
  
  void stopWaiting();
  void startWaiting();
  
public:
  static Ptr create(GtkProgressBar* progressBar=NULL);

  ~ProgressBarManager();

  void setProgressBar(GtkProgressBar* progressBar);

  // ProgressInterface ///////////////////////////////////////////////////
  
  virtual void setIdle();
  virtual void setWaiting(double progress=0.0);
  virtual void setWorking(double progress);
  virtual void setFinished();
};

#endif
