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

#ifndef THREADPOOLIMPL_HH
#define THREADPOOLIMPL_HH

#include <queue>

#include <boost/thread.hpp>
#include <boost/function.hpp>

#include <scroom/semaphore.hh>

#include <scroom/workinterface.hh>

class ThreadPool
{
private:
  std::list<boost::thread*> threads;
  Scroom::Semaphore jobcount;
  std::map<int, std::queue<WorkInterface*> > jobs;
  boost::mutex mut;
  boost::mutex threadsMut;
  bool alive;

private:
  void work();
  bool perform_one();
  
public:
  ThreadPool();
  ThreadPool(int count);
  ~ThreadPool();
  void schedule(int priority, WorkInterface* wi);
  void schedule(int priority, boost::function<void ()> const& fn);
  void schedule_on_new_thread(WorkInterface* wi);
  void cleanUp();
};

#endif
