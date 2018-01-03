/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "progressbarmanager.hh"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <scroom/assertions.hh>
#include <scroom/gtk-helpers.hh>

#include "workinterface.hh"
#include "callbacks.hh"

namespace
{
  class ProgressBarPulser : public WorkInterface
  {
  public:
    typedef boost::shared_ptr<ProgressBarPulser> Ptr;

  private:
    boost::mutex mut;
    std::list<GtkProgressBar*> progressbars;
    std::list<GtkProgressBar*>::iterator current;

  public:
    void start(GtkProgressBar* progressBar);
    void stop(GtkProgressBar* progressBar);

    // WorkInterface ///////////////////////////////////////////////////////

    virtual bool doWork();
  };

  ////////////////////////////////////////////////////////////////////////
  // Regular functions
  
  ProgressBarPulser::Ptr instance()
  {
    static ProgressBarPulser::Ptr pulser = ProgressBarPulser::Ptr(new ProgressBarPulser());
    return pulser;
  }

  ////////////////////////////////////////////////////////////////////////
  // ProgressBarPulser

  void ProgressBarPulser::start(GtkProgressBar* progressBar)
  {
    boost::mutex::scoped_lock lock(mut);
      
    progressbars.push_back(progressBar);

    if(progressbars.size()==1)
    {
      current = progressbars.begin();
      g_timeout_add(100, on_idle, static_cast<WorkInterface*>(this));
    }
  }
  
  void ProgressBarPulser::stop(GtkProgressBar* progressBar)
  {
    boost::mutex::scoped_lock lock(mut);

    BOOST_FOREACH(GtkProgressBar* &p, progressbars)
    {
      if(p==progressBar)
        p=NULL;
    }
  }
  
  bool ProgressBarPulser::doWork()
  {
    // Locking these the other way around results in a deadlock. See ticket #40
    Scroom::GtkHelpers::TakeGdkLock gdkLock;
    boost::mutex::scoped_lock lock(mut);

    while(current == progressbars.end() || *current==NULL)
    {
      if(progressbars.empty())
      {
        return false;
      }
      else if(current == progressbars.end())
        current = progressbars.begin();
      else if (*current == NULL)
        current = progressbars.erase(current);
      else
      {
        defect();
      }
    }

    gtk_progress_bar_pulse(*current);
    ++current;
    
    return true;
  }
}

ProgressBarManager::ProgressBarManager(GtkProgressBar* progressBar)
  :progressBar(progressBar), isWaiting(false)
{}

ProgressBarManager::Ptr ProgressBarManager::create(GtkProgressBar* progressBar)
{
  return Ptr(new ProgressBarManager(progressBar));
}

ProgressBarManager::~ProgressBarManager()
{
  stopWaiting();
}

void ProgressBarManager::setProgressBar(GtkProgressBar* progressBar)
{
  stopWaiting();
  this->progressBar = progressBar;
}

void ProgressBarManager::startWaiting()
{
  if(!isWaiting)
  {
    // Start pulsing
    instance()->start(progressBar);
    isWaiting = true;
  }
}  

void ProgressBarManager::stopWaiting()
{
  if(isWaiting)
  {
    // Stop pulsing
    instance()->stop(progressBar);
    isWaiting = false;
  }
}  


// ProgressInterface ///////////////////////////////////////////////////

void ProgressBarManager::setIdle()
{
  setWorking(0.0);
}

void ProgressBarManager::setWaiting(double)
{
  startWaiting();
}

void ProgressBarManager::setWorking(double progress)
{
  stopWaiting();

  Scroom::GtkHelpers::TakeGdkLock gdkLock;
  gtk_progress_bar_set_fraction(progressBar, progress);
}

void ProgressBarManager::setFinished()
{
  setIdle();
}
