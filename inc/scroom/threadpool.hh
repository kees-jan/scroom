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

#ifndef _THREADPOOL_HH
#define _THREADPOOL_HH

#include <boost/thread.hpp>

#include <scroom/semaphore.hh>
#include <scroom/workinterface.hh>
#include <scroom/workinterface.hh>

enum
  {
    PRIO_HIGHEST = 100,
    PRIO_HIGHER,
    PRIO_HIGH,
    PRIO_NORMAL,
    PRIO_LOW,
    PRIO_LOWER,
    PRIO_LOWEST,
  };

class Wrapper : public WorkInterface
{
private:
  WorkInterface* wi;

public:
  Wrapper(WorkInterface* wi)
    : wi(wi)
  {}

  virtual bool doWork()
  {
    return wi->doWork();
  }
};

class QueueJumper : private WorkInterface
{
private:
  WorkInterface* wi;
  boost::mutex mut;
  bool inQueue;
  int priority;
  Scroom::Semaphore done;

public:
  QueueJumper();

  bool setWork(WorkInterface* wi);
  bool isDone();
  void waitForDone();
  void asynchronousCleanup();

  void schedule(int priority=PRIO_NORMAL);
  
private:
  virtual bool doWork();
};


/**
 * Abstraction of a sequential job.
 *
 * Jobs of this type are scheduled using the sequentially()
 * function.
 */
class SeqJob : public WorkInterface
{
public:
  virtual ~SeqJob() {}

protected:
  virtual void done();
};

/**
 * Schedule jobs to be executed in parallel.
 *
 * Jobs with the highest priority will be scheduled first. However,
 * since there are multiple threads executing jobs, there is no
 * guarantee that no lower priority jobs will be started before the
 * last high-priority job is finished. You should do your own
 * synchronisation :-)
 *
 * After your job completes, it will be deleted.
 *
 * @bug Jobs of the same priority are not executed in order.
 */
void schedule(WorkInterface* wi, int priority=PRIO_NORMAL);

/**
 * Schedule jobs to be executed sequentially.
 *
 * When it is your job's time to run, its WorkInterface::doWork()
 * function will be called. When your job is done, it should call its
 * done() member function, to signal that the next job can run. After
 * you call done, your job will be deleted.
 *
 * Calling your own done() member doesn't really make sense any
 * more. It used to be that the WorkInterface::doWork() method was
 * called on the gtk thread. Hence, it couldn't really do any work
 * itself, but had to schedule() one or more jobs on the threadpool
 * instead. Then, when all those jobs were done, you'd need to call
 * your done() member function.
 *
 * Currently, sequentially() executes its jobs on its own thread, so
 * it is allowed to do actual work in you WorkInterface::doWork()
 * method.
 *
 * @todo I should one day refactor this :-)
 */
void sequentially(SeqJob* job);

void schedule_on_new_thread(WorkInterface* wi);

#endif
