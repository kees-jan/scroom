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
#include <scroom/progressinterfacemultiplexer.hh>

#include <boost/foreach.hpp>

using namespace Scroom::Utils;

////////////////////////////////////////////////////////////////////////

ProgressInterfaceMultiplexer::ChildData::ChildData()
  : state(ProgressStateInterface::IDLE), progress(0.0)
{}

ProgressInterfaceMultiplexer::ChildData::Ptr ProgressInterfaceMultiplexer::ChildData::create()
{
  return Ptr(new ChildData());
}

void ProgressInterfaceMultiplexer::ChildData::setProgress(State state, double progress)
{
  boost::mutex::scoped_lock l(mut);
  this->state = state;
  this->progress = progress;
}  

void ProgressInterfaceMultiplexer::ChildData::clearFinished()
{
  boost::mutex::scoped_lock l(mut);
  if(state == FINISHED)
  {
    state = IDLE;
    progress = 0.0;
  }
}  

////////////////////////////////////////////////////////////////////////

ProgressInterfaceMultiplexer::Child::Child(ProgressInterfaceMultiplexer::Ptr parent, ChildData::Ptr data)
  : parent(parent), data(data)
{}

ProgressInterfaceMultiplexer::Child::~Child()
{
  parent->unsubscribe(data);
  parent->updateProgressState();
}

ProgressInterfaceMultiplexer::Child::Ptr ProgressInterfaceMultiplexer::Child::create(ProgressInterfaceMultiplexer::Ptr parent, ChildData::Ptr data)
{
  return Ptr(new Child(parent, data));
}

void ProgressInterfaceMultiplexer::Child::setProgress(State state, double progress)
{
  data->setProgress(state, progress);
  parent->updateProgressState();
}

////////////////////////////////////////////////////////////////////////

ProgressInterfaceMultiplexer::ProgressInterfaceMultiplexer(ProgressInterface::Ptr parent)
  : parent(ProgressStateInterfaceFromProgressInterfaceForwarder::create(parent))
{}

ProgressInterfaceMultiplexer::Ptr ProgressInterfaceMultiplexer::create(ProgressInterface::Ptr parent)
{
  return Ptr(new ProgressInterfaceMultiplexer(parent));
}

ProgressInterface::Ptr ProgressInterfaceMultiplexer::createProgressInterface()
{
  ChildData::Ptr data = ChildData::create();
  Child::Ptr child = Child::create(shared_from_this<ProgressInterfaceMultiplexer>(), data);

  boost::mutex::scoped_lock l(mut);
  children.insert(data);

  return child;
}

void ProgressInterfaceMultiplexer::unsubscribe(ChildData::Ptr data)
{
  boost::mutex::scoped_lock l(mut);
  children.erase(data);
}

void ProgressInterfaceMultiplexer::updateProgressState()
{
  ProgressStateInterface::State state = ProgressStateInterface::IDLE;
  double progress = 0.0;
  int workers = 0;

  boost::mutex::scoped_lock l(mut);
  BOOST_FOREACH(const ChildData::Ptr& child, children)
  {
    boost::mutex::scoped_lock child_lock(child->mut);
    switch(child->state)
    {
    case ProgressStateInterface::IDLE:
      break;
    case ProgressStateInterface::WAITING:
      if(state==ProgressStateInterface::IDLE || state==ProgressStateInterface::FINISHED)
        state = ProgressStateInterface::WAITING;
      progress+=child->progress;
      workers++;
      break;
    case ProgressStateInterface::WORKING:
      if(state!=ProgressStateInterface::WORKING)
        state = ProgressStateInterface::WORKING;
      progress+=child->progress;
      workers++;
      break;
    case ProgressStateInterface::FINISHED:
      if(state==ProgressStateInterface::IDLE)
        state=ProgressStateInterface::FINISHED;
      progress+=1.0;
      workers++;
      break;
    }
  }

  parent->setProgress(state, progress/workers);

  if(state==ProgressStateInterface::FINISHED)
  {
    BOOST_FOREACH(const ChildData::Ptr& child, children)
    {
      child->clearFinished();
    }
  }
}
