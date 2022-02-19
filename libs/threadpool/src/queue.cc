/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "queue.hh"

using namespace Scroom::Detail::ThreadPool;

////////////////////////////////////////////////////////////////////////
/// QueueImpl
////////////////////////////////////////////////////////////////////////

QueueImpl::Ptr QueueImpl::create() { return QueueImpl::Ptr(new QueueImpl()); }

void QueueImpl::deletingQueue()
{
  boost::mutex::scoped_lock lock(mut);
  isDeleted = true;
  while(count != 0)
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
  : q(queue)
  , isValid(q->jobStarted())
{
}

bool QueueLock::queueExists() const { return isValid; }

QueueLock::~QueueLock() { q->jobFinished(); }
