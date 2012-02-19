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
#include "queue.hh"

using namespace Scroom::Detail::ThreadPool;

////////////////////////////////////////////////////////////////////////
/// QueueImpl
////////////////////////////////////////////////////////////////////////

QueueImpl::Ptr QueueImpl::create()
{
  return QueueImpl::Ptr(new QueueImpl());
}

QueueImpl::QueueImpl()
: mut(), cond(), count(0), isDeleted(false)
{
}

void QueueImpl::deletingQueue()
{
  boost::mutex::scoped_lock lock(mut);
  isDeleted = true;
  while(count!=0)
  {
    cond.wait(lock);
  }
}

bool QueueImpl::jobStarted()
{
  boost::mutex::scoped_lock lock(mut);
  count++;

  return !isDeleted;
}

void QueueImpl::jobFinished()
{
  boost::mutex::scoped_lock lock(mut);
  count--;
  cond.notify_all();
}

int QueueImpl::getCount()
{
  boost::mutex::scoped_lock lock(mut);
  return count;
}


////////////////////////////////////////////////////////////////////////
/// QueueLock
////////////////////////////////////////////////////////////////////////

QueueLock::QueueLock(QueueImpl::Ptr queue)
:q(queue), isValid(q->jobStarted())
{
}

bool QueueLock::queueExists()
{
  return isValid;
}

QueueLock::~QueueLock()
{
  q->jobFinished();
}
