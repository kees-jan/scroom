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

      void ProgressStore::setState(State s)
      {
        state=s;
      }
      
      void ProgressStore::setProgress(double p)
      {
        progress = p;
      }
        
      void ProgressStore::setProgress(int done, int total)
      {
        setProgress(double(done)/total);
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

    void ProgressInterfaceBroadcaster::setWorking(int done, int total)
    {
      setWorking(double(done)/total);
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
