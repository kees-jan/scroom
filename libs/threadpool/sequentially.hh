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

#ifndef SEQUENTIALLY_HH
#define SEQUENTIALLY_HH

#include <list>

#include <boost/thread/mutex.hpp>

#include <threadpool.hh>

class Sequentially
{
private:
  boost::mutex remainingJobsMutex;
  std::list<SeqJob*> remainingJobs;
  bool currentlyWorking;

  friend class SeqJob;
  
public:
  static Sequentially& getInstance();
  
public:
  Sequentially();
  ~Sequentially();

  void execute(SeqJob* job);

  // Helpers /////////////////////////////////////////////////////////////
private:
  void done();

public:
  void do_next();
};

#endif
