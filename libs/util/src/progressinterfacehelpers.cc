/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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
#include <scroom/progressinterfacehelpers.hh>

using namespace Scroom::Utils;

////////////////////////////////////////////////////////////////////////

void ProgressInterfaceFromProgressStateInterface::setIdle()
{
  setState(IDLE);
}

void ProgressInterfaceFromProgressStateInterface::setWaiting(double progress)
{
  setState(WAITING);
  setProgress(progress);
}

void ProgressInterfaceFromProgressStateInterface::setWorking(double progress)
{
  setState(WORKING);
  setProgress(progress);
}

void ProgressInterfaceFromProgressStateInterface::setWorking(int done, int total)
{
  setState(WORKING);
  setProgress(done, total);
}

void ProgressInterfaceFromProgressStateInterface::setFinished()
{
  setState(FINISHED);
  setProgress(1.0);
}

////////////////////////////////////////////////////////////////////////

ProgressInterfaceFromProgressStateInterfaceForwarder::ProgressInterfaceFromProgressStateInterfaceForwarder(ProgressStateInterface::Ptr child)
  : child(child)
{
}

ProgressInterfaceFromProgressStateInterfaceForwarder::Ptr ProgressInterfaceFromProgressStateInterfaceForwarder::create(ProgressStateInterface::Ptr child)
{
  return Ptr(new ProgressInterfaceFromProgressStateInterfaceForwarder(child));
}

void ProgressInterfaceFromProgressStateInterfaceForwarder::setState(State s)
{
  child->setState(s);
}

void ProgressInterfaceFromProgressStateInterfaceForwarder::setProgress(double progress)
{
  child->setProgress(progress);
}

void ProgressInterfaceFromProgressStateInterfaceForwarder::setProgress(int done, int total)
{
  child->setProgress(done, total);
}

////////////////////////////////////////////////////////////////////////

ProgressStateInterfaceFromProgressInterface::ProgressStateInterfaceFromProgressInterface()
  : progress(0.0)
{}
      
void ProgressStateInterfaceFromProgressInterface::setState(State s)
{
  switch(s)
  {
  case IDLE:
    setIdle();
    break;
  case WAITING:
    setWaiting(progress);
    break;
  case WORKING:
    setWorking(progress);
    break;
  case FINISHED:
    setFinished();
    break;
  }
}

void ProgressStateInterfaceFromProgressInterface::setProgress(double progress_)
{
  progress = progress_;
  setWorking(progress);
}
    
void ProgressStateInterfaceFromProgressInterface::setProgress(int done, int total)
{
  progress = 1.0 * done / total;
  setWorking(done, total);
}

////////////////////////////////////////////////////////////////////////

ProgressStateInterfaceFromProgressInterfaceForwarder::ProgressStateInterfaceFromProgressInterfaceForwarder(ProgressInterface::Ptr child)
  : child(child)
{
}

ProgressStateInterfaceFromProgressInterfaceForwarder::Ptr ProgressStateInterfaceFromProgressInterfaceForwarder::create(ProgressInterface::Ptr child)
{
  return Ptr(new ProgressStateInterfaceFromProgressInterfaceForwarder(child));
}

void ProgressStateInterfaceFromProgressInterfaceForwarder::setIdle()
{
  child->setIdle();
}

void ProgressStateInterfaceFromProgressInterfaceForwarder::setWaiting(double progress)
{
  child->setWaiting(progress);
}

void ProgressStateInterfaceFromProgressInterfaceForwarder::setWorking(double progress)
{
  child->setWorking(progress);
}

void ProgressStateInterfaceFromProgressInterfaceForwarder::setWorking(int done, int total)
{
  child->setWorking(done, total);
}

void ProgressStateInterfaceFromProgressInterfaceForwarder::setFinished()
{
  child->setFinished();
}
