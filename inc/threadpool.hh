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

#include <workinterface.hh>

enum
  {
    PRIO_LOWEST=100,
    PRIO_LOWER,
    PRIO_LOW,
    PRIO_NORMAL,
    PRIO_HIGH,
    PRIO_HIGHER,
    PRIO_HIGHEST
  };

class SeqJob : public WorkInterface
{
public:
  virtual ~SeqJob() {}

protected:
  virtual void done();
};

void schedule(WorkInterface* wi, int priority=PRIO_NORMAL);

void sequentially(SeqJob* job);

#endif
