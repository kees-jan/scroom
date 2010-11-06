/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

#include "sequentially.hh"

#include <gtk/gtk.h>

#include <unused.h>

////////////////////////////////////////////////////////////////////////

gboolean next(gpointer data)
{
  UNUSED(data);
  Sequentially::getInstance().do_next();

  return FALSE;
}

////////////////////////////////////////////////////////////////////////

void SeqJob::done()
{
  Sequentially::getInstance().done();
}

////////////////////////////////////////////////////////////////////////

void sequentially(SeqJob* job)
{
  Sequentially::getInstance().execute(job);
}

////////////////////////////////////////////////////////////////////////

Sequentially& Sequentially::getInstance()
{
  static Sequentially instance;
  return instance;
}

Sequentially::Sequentially()
  : currentlyWorking(false)
{
}

Sequentially::~Sequentially()
{
  if(currentlyWorking)
    printf("ERROR: Still working!\n");
}

void Sequentially::execute(SeqJob* job)
{
  boost::mutex::scoped_lock lock(remainingJobsMutex);
  remainingJobs.push_back(job);
  if(!currentlyWorking)
  {
    currentlyWorking = true;
    gdk_threads_add_idle(next, this);
  }
}

// Helpers /////////////////////////////////////////////////////////////

void Sequentially::done()
{
  boost::mutex::scoped_lock lock(remainingJobsMutex);
  SeqJob* job = remainingJobs.front();
  delete job;
  remainingJobs.pop_front();
  gdk_threads_add_idle(next, this);
}

void Sequentially::do_next()
{
  SeqJob* job = NULL;

  { // New scope for scoped lock
    boost::mutex::scoped_lock lock(remainingJobsMutex);
    if(!remainingJobs.empty())
    {
      job = remainingJobs.front();
      currentlyWorking=true;
    }
    else
      currentlyWorking=false;
  }

  if(job)
  {
    job->doWork();
  }
}
