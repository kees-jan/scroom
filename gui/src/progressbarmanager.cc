/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "progressbarmanager.hh"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

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
    ProgressBarPulser();
    
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

  ProgressBarPulser::ProgressBarPulser()
  {}

  void ProgressBarPulser::start(GtkProgressBar* progressBar)
  {
    boost::unique_lock<boost::mutex> lock(mut);
      
    progressbars.push_back(progressBar);

    if(progressbars.size()==1)
    {
      current = progressbars.begin();
      g_timeout_add(100, on_idle, static_cast<WorkInterface*>(this));
    }
  }
  
  void ProgressBarPulser::stop(GtkProgressBar* progressBar)
  {
    boost::unique_lock<boost::mutex> lock(mut);

    BOOST_FOREACH(GtkProgressBar* &p, progressbars)
    {
      if(p==progressBar)
        p=NULL;
    }
  }
  
  bool ProgressBarPulser::doWork()
  {
    // Locking these the other way around results in a deadlock. See ticket #40
    gdk_threads_enter();
    boost::unique_lock<boost::mutex> lock(mut);

    while(current == progressbars.end() || *current==NULL)
    {
      if(progressbars.empty())
      {
        gdk_threads_leave();
        return false;
      }
      else if(current == progressbars.end())
        current = progressbars.begin();
      else if (*current == NULL)
        current = progressbars.erase(current);
      else
      {
        printf("PANIC: Shoudn't happen\n");
        abort();
      }
    }

    gtk_progress_bar_pulse(*current);
    gdk_threads_leave();
    ++current;
    
    return true;
  }
}

// void TiledBitmapViewData::gtk_progress_bar_set_fraction(double fraction)
// {
//   ::gtk_progress_bar_set_fraction(progressBar, fraction);
// }
// 
// void TiledBitmapViewData::gtk_progress_bar_pulse()
// {
//   ::gtk_progress_bar_pulse(progressBar);
// }

ProgressBarManager::ProgressBarManager(GtkProgressBar* progressBar)
  :progressBar(progressBar), state(IDLE)
{}

ProgressBarManager::~ProgressBarManager()
{
  if(state == WAITING)
  {
    // Stop pulsing
    instance()->stop(progressBar);
    state = IDLE;
  }
}

void ProgressBarManager::setProgressBar(GtkProgressBar* progressBar)
{
  this->progressBar = progressBar;
}
  
// ProgressInterface ///////////////////////////////////////////////////
  
void ProgressBarManager::setState(State s)
{
  if(state != s)
  {
    if(state == WAITING)
    {
      // s != WAITING, stop pulsing
      instance()->stop(progressBar);
    }
    if(s == WAITING)
    {
      // state != WAITING, start pulsing
      instance()->start(progressBar);
    }
    switch(s)
    {
    case IDLE:
    case FINISHED:
      gtk_progress_bar_set_fraction(progressBar, 0.0);
      break;
    case WAITING:
      // already handled
      break;
    case WORKING:
      // will be handled by calls to setProgress()
      break;
    default:
      // Panic, shouldn't happen
      break;
    }
    state = s;
  }
}

void ProgressBarManager::setProgress(double d)
{
  setState(ProgressInterface::WORKING);
  gtk_progress_bar_set_fraction(progressBar, d);  
}

void ProgressBarManager::setProgress(int done, int total)
{
  setProgress(1.0*done/total);
}
