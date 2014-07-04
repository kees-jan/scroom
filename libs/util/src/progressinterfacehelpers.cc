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
    
  } // namespace Scroom::Utils
} // namespace Scroom
