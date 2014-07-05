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

#include <boost/foreach.hpp>

#include <scroom/unused.hh>
#include <scroom/assertions.hh>

namespace Scroom
{
  namespace Utils
  {

    ////////////////////////////////////////////////////////////////////////

    void ProgressInterfaceFromProgressStateInterface::setIdle()
    {
      setProgress(IDLE);
    }

    void ProgressInterfaceFromProgressStateInterface::setWaiting(double progress)
    {
      setProgress(WAITING, progress);
    }

    void ProgressInterfaceFromProgressStateInterface::setWorking(double progress)
    {
      setProgress(WORKING, progress);
    }

    void ProgressInterfaceFromProgressStateInterface::setFinished()
    {
      setProgress(FINISHED, 1.0);
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

    void ProgressInterfaceFromProgressStateInterfaceForwarder::setProgress(State s, double progress)
    {
      child->setProgress(s, progress);
    }

    ////////////////////////////////////////////////////////////////////////

    void ProgressStateInterfaceFromProgressInterface::setProgress(State s, double progress)
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

    void ProgressStateInterfaceFromProgressInterfaceForwarder::setFinished()
    {
      child->setFinished();
    }

    ////////////////////////////////////////////////////////////////////////

    namespace Detail
    {
      ProgressStore::Ptr ProgressStore::create()
      {
        return Ptr(new ProgressStore());
      }

      ProgressStore::ProgressStore()
        : state(IDLE), progress(0.0)
      {}

      void ProgressStore::init(ProgressInterface::Ptr const& i)
      {
        switch(state)
        {
        case IDLE: i->setIdle(); break;
        case WAITING: i->setWaiting(progress); break;
        case WORKING: i->setWorking(progress); break;
        case FINISHED: i->setFinished(); break;
        }
      }

      void ProgressStore::setProgress(State s, double p)
      {
        state=s;
        progress = p;
      }
    }
    ////////////////////////////////////////////////////////////////////////

    ProgressInterfaceBroadcaster::Ptr ProgressInterfaceBroadcaster::create()
    {
      return Ptr(new ProgressInterfaceBroadcaster());
    }

    ProgressInterfaceBroadcaster::ProgressInterfaceBroadcaster()
    {
      store = Detail::ProgressStore::create();
      children.insert(store);
    }

    Stuff ProgressInterfaceBroadcaster::subscribe(ProgressInterface::Ptr const& child)
    {
      boost::mutex::scoped_lock lock(mut);
      store->init(child);
      children.insert(child);
      return Unsubscriber::create(shared_from_this<ProgressInterfaceBroadcaster>(), child);
    }

    void ProgressInterfaceBroadcaster::unsubscribe(ProgressInterface::Ptr const& child)
    {
      boost::mutex::scoped_lock lock(mut);
      children.erase(child);
    }

    void ProgressInterfaceBroadcaster::setIdle()
    {
      boost::mutex::scoped_lock lock(mut);
      BOOST_FOREACH(ProgressInterface::Ptr const& child, children)
        child->setIdle();
    }

    void ProgressInterfaceBroadcaster::setWaiting(double progress)
    {
      boost::mutex::scoped_lock lock(mut);
      BOOST_FOREACH(ProgressInterface::Ptr const& child, children)
        child->setWaiting(progress);
    }

    void ProgressInterfaceBroadcaster::setWorking(double progress)
    {
      boost::mutex::scoped_lock lock(mut);
      BOOST_FOREACH(ProgressInterface::Ptr const& child, children)
        child->setWorking(progress);
    }

    void ProgressInterfaceBroadcaster::setFinished()
    {
      boost::mutex::scoped_lock lock(mut);
      BOOST_FOREACH(ProgressInterface::Ptr const& child, children)
        child->setFinished();
    }

    ////////////////////////////////////////////////////////////////////////

    ProgressInterfaceBroadcaster::Unsubscriber::Ptr
    ProgressInterfaceBroadcaster::Unsubscriber::create(ProgressInterfaceBroadcaster::Ptr const& parent,
                                                         ProgressInterface::Ptr const& child)
    {
      return Ptr(new ProgressInterfaceBroadcaster::Unsubscriber(parent, child));
    }

    ProgressInterfaceBroadcaster::Unsubscriber::Unsubscriber(ProgressInterfaceBroadcaster::Ptr const& parent,
                                                               ProgressInterface::Ptr const& child)
      : parent(parent), child(child)
    {
    }
    
    ProgressInterfaceBroadcaster::Unsubscriber::~Unsubscriber()
    {
      parent->unsubscribe(child);
    }

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
    
  } // namespace Scroom::Utils
} // namespace Scroom
