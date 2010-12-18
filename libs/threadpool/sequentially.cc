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

#include <stdio.h>
#include <scroom/unused.h>

////////////////////////////////////////////////////////////////////////

static Sequentially& instance()
{
  static Sequentially instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////

void SeqJob::done()
{
  instance().done();
}

////////////////////////////////////////////////////////////////////////

void sequentially(SeqJob* job)
{
  instance().execute(job);
}

////////////////////////////////////////////////////////////////////////

Sequentially::Sequentially()
  : currentlyWorking(false), asynchronous(1)
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
    asynchronous.schedule(boost::bind(&Sequentially::next, this), PRIO_NORMAL);
  }
}

// Helpers /////////////////////////////////////////////////////////////

void Sequentially::done()
{
  boost::mutex::scoped_lock lock(remainingJobsMutex);
  SeqJob* job = remainingJobs.front();
  delete job;
  remainingJobs.pop_front();
  asynchronous.schedule(boost::bind(&Sequentially::next, this), PRIO_NORMAL);
}

void Sequentially::next()
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
