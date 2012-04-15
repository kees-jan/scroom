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
  : state(ProgressInterface::IDLE), progress(0.0)
{}

ProgressInterfaceMultiplexer::ChildData::Ptr ProgressInterfaceMultiplexer::ChildData::create()
{
  return Ptr(new ChildData());
}

////////////////////////////////////////////////////////////////////////

ProgressInterfaceMultiplexer::Child::Child(ProgressInterfaceMultiplexer::Ptr parent, ChildData::Ptr data)
  : parent(parent), data(data)
{}

ProgressInterfaceMultiplexer::Child::Ptr ProgressInterfaceMultiplexer::Child::create(ProgressInterfaceMultiplexer::Ptr parent, ChildData::Ptr data)
{
  return Ptr(new Child(parent, data));
}

void ProgressInterfaceMultiplexer::Child::setState(State state)
{
  data->state = state;
  parent->updateProgressState();
}

void ProgressInterfaceMultiplexer::Child::setProgress(double progress)
{
  data->progress = progress;
  parent->updateProgressState();
}

void ProgressInterfaceMultiplexer::Child::setProgress(int done, int total)
{
  setProgress(1.0*done/total);
}

////////////////////////////////////////////////////////////////////////

ProgressInterfaceMultiplexer::ProgressInterfaceMultiplexer(ProgressInterface::Ptr parent)
  : parent(parent)
{}

ProgressInterfaceMultiplexer::Ptr ProgressInterfaceMultiplexer::create(ProgressInterface::Ptr parent)
{
  return Ptr(new ProgressInterfaceMultiplexer(parent));
}

ProgressInterface::Ptr ProgressInterfaceMultiplexer::createProgressInterface()
{
  ChildData::Ptr data = ChildData::create();
  Child::Ptr child = Child::create(shared_from_this<ProgressInterfaceMultiplexer>(), data);

  children.push_back(data);

  return child;
}

void ProgressInterfaceMultiplexer::updateProgressState()
{
  ProgressInterface::State state = ProgressInterface::IDLE;
  double progress = 0.0;
  int workers = 0;
  
  BOOST_FOREACH(const ChildData::Ptr& child, children)
  {
    switch(child->state)
    {
    case ProgressInterface::IDLE:
      break;
    case ProgressInterface::WAITING:
      if(state==ProgressInterface::IDLE)
        state = ProgressInterface::WAITING;
      progress+=child->progress;
      workers++;
      break;
    case ProgressInterface::WORKING:
      if(state!=ProgressInterface::WORKING)
        state = ProgressInterface::WORKING;
      progress+=child->progress;
      workers++;
      break;
    case ProgressInterface::FINISHED:
      if(state==ProgressInterface::IDLE)
        state=ProgressInterface::FINISHED;
      progress+=1.0;
      workers++;
      break;
    }

    parent->setProgress(progress/workers);
    parent->setState(state);
  }
}
