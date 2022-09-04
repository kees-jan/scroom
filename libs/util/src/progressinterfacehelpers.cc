/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <utility>

#include <scroom/assertions.hh>
#include <scroom/progressinterfacehelpers.hh>
#include <scroom/unused.hh>

namespace Scroom::Utils
{

  ////////////////////////////////////////////////////////////////////////

  void ProgressInterfaceFromProgressStateInterface::setIdle() { setProgress(IDLE); }

  void ProgressInterfaceFromProgressStateInterface::setWaiting(double progress) { setProgress(WAITING, progress); }

  void ProgressInterfaceFromProgressStateInterface::setWorking(double progress) { setProgress(WORKING, progress); }

  void ProgressInterfaceFromProgressStateInterface::setFinished() { setProgress(FINISHED, 1.0); }

  ////////////////////////////////////////////////////////////////////////

  ProgressInterfaceFromProgressStateInterfaceForwarder::ProgressInterfaceFromProgressStateInterfaceForwarder(
    ProgressStateInterface::Ptr child_)
    : child(std::move(child_))
  {
  }

  ProgressInterfaceFromProgressStateInterfaceForwarder::Ptr
    ProgressInterfaceFromProgressStateInterfaceForwarder::create(ProgressStateInterface::Ptr child)
  {
    return Ptr(new ProgressInterfaceFromProgressStateInterfaceForwarder(std::move(child)));
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

  ProgressStateInterfaceFromProgressInterfaceForwarder::ProgressStateInterfaceFromProgressInterfaceForwarder(
    ProgressInterface::Ptr child_)
    : child(std::move(child_))
  {
  }

  ProgressStateInterfaceFromProgressInterfaceForwarder::Ptr
    ProgressStateInterfaceFromProgressInterfaceForwarder::create(ProgressInterface::Ptr child)
  {
    return Ptr(new ProgressStateInterfaceFromProgressInterfaceForwarder(std::move(child)));
  }

  void ProgressStateInterfaceFromProgressInterfaceForwarder::setIdle() { child->setIdle(); }

  void ProgressStateInterfaceFromProgressInterfaceForwarder::setWaiting(double progress) { child->setWaiting(progress); }

  void ProgressStateInterfaceFromProgressInterfaceForwarder::setWorking(double progress) { child->setWorking(progress); }

  void ProgressStateInterfaceFromProgressInterfaceForwarder::setFinished() { child->setFinished(); }

  ////////////////////////////////////////////////////////////////////////

  namespace Detail
  {
    ProgressStore::Ptr ProgressStore::create() { return Ptr(new ProgressStore()); }

    void ProgressStore::init(ProgressInterface::Ptr const& i)
    {
      switch(state)
      {
      case IDLE:
        i->setIdle();
        break;
      case WAITING:
        i->setWaiting(progress);
        break;
      case WORKING:
        i->setWorking(progress);
        break;
      case FINISHED:
        i->setFinished();
        break;
      }
    }

    void ProgressStore::setProgress(State s, double p)
    {
      state    = s;
      progress = p;
    }
  } // namespace Detail
  ////////////////////////////////////////////////////////////////////////

  ProgressInterfaceBroadcaster::Ptr ProgressInterfaceBroadcaster::create() { return Ptr(new ProgressInterfaceBroadcaster()); }

  ProgressInterfaceBroadcaster::ProgressInterfaceBroadcaster()
  {
    store = Detail::ProgressStore::create();
    children.insert(store);
  }

  Stuff ProgressInterfaceBroadcaster::subscribe(ProgressInterface::Ptr const& child)
  {
    boost::mutex::scoped_lock const lock(mut);
    store->init(child);
    children.insert(child);
    return Unsubscriber::create(shared_from_this<ProgressInterfaceBroadcaster>(), child);
  }

  void ProgressInterfaceBroadcaster::unsubscribe(ProgressInterface::Ptr const& child)
  {
    boost::mutex::scoped_lock const lock(mut);
    children.erase(child);
  }

  void ProgressInterfaceBroadcaster::setIdle()
  {
    boost::mutex::scoped_lock const lock(mut);
    for(ProgressInterface::Ptr const& child: children)
    {
      child->setIdle();
    }
  }

  void ProgressInterfaceBroadcaster::setWaiting(double progress)
  {
    boost::mutex::scoped_lock const lock(mut);
    for(ProgressInterface::Ptr const& child: children)
    {
      child->setWaiting(progress);
    }
  }

  void ProgressInterfaceBroadcaster::setWorking(double progress)
  {
    boost::mutex::scoped_lock const lock(mut);
    for(ProgressInterface::Ptr const& child: children)
    {
      child->setWorking(progress);
    }
  }

  void ProgressInterfaceBroadcaster::setFinished()
  {
    boost::mutex::scoped_lock const lock(mut);
    for(ProgressInterface::Ptr const& child: children)
    {
      child->setFinished();
    }
  }

  ////////////////////////////////////////////////////////////////////////

  ProgressInterfaceBroadcaster::Unsubscriber::Ptr
    ProgressInterfaceBroadcaster::Unsubscriber::create(ProgressInterfaceBroadcaster::Ptr const& parent,
                                                       ProgressInterface::Ptr const&            child)
  {
    return Ptr(new ProgressInterfaceBroadcaster::Unsubscriber(parent, child));
  }

  ProgressInterfaceBroadcaster::Unsubscriber::Unsubscriber(ProgressInterfaceBroadcaster::Ptr const& parent_,
                                                           ProgressInterface::Ptr const&            child_)
    : parent(parent_)
    , child(child_)
  {
  }

  ProgressInterfaceBroadcaster::Unsubscriber::~Unsubscriber() { parent->unsubscribe(child); }

  ////////////////////////////////////////////////////////////////////////

  ProgressInterfaceMultiplexer::ChildData::Ptr ProgressInterfaceMultiplexer::ChildData::create() { return Ptr(new ChildData()); }

  void ProgressInterfaceMultiplexer::ChildData::setProgress(State state_, double progress_)
  {
    boost::mutex::scoped_lock const l(mut);
    state    = state_;
    progress = progress_;
  }

  void ProgressInterfaceMultiplexer::ChildData::clearFinished()
  {
    boost::mutex::scoped_lock const l(mut);
    if(state == FINISHED)
    {
      state    = IDLE;
      progress = 0.0;
    }
  }

  ////////////////////////////////////////////////////////////////////////

  ProgressInterfaceMultiplexer::Child::Child(ProgressInterfaceMultiplexer::Ptr parent_, ChildData::Ptr data_)
    : parent(std::move(parent_))
    , data(std::move(data_))
  {
  }

  ProgressInterfaceMultiplexer::Child::~Child()
  {
    parent->unsubscribe(data);
    parent->updateProgressState();
  }

  ProgressInterfaceMultiplexer::Child::Ptr ProgressInterfaceMultiplexer::Child::create(ProgressInterfaceMultiplexer::Ptr parent,
                                                                                       ChildData::Ptr                    data)
  {
    return Ptr(new Child(std::move(parent), std::move(data)));
  }

  void ProgressInterfaceMultiplexer::Child::setProgress(State state, double progress)
  {
    data->setProgress(state, progress);
    parent->updateProgressState();
  }

  ////////////////////////////////////////////////////////////////////////

  ProgressInterfaceMultiplexer::ProgressInterfaceMultiplexer(ProgressInterface::Ptr parent_)
    : parent(ProgressStateInterfaceFromProgressInterfaceForwarder::create(std::move(parent_)))
  {
  }

  ProgressInterfaceMultiplexer::Ptr ProgressInterfaceMultiplexer::create(ProgressInterface::Ptr parent)
  {
    return Ptr(new ProgressInterfaceMultiplexer(std::move(parent)));
  }

  ProgressInterface::Ptr ProgressInterfaceMultiplexer::createProgressInterface()
  {
    ChildData::Ptr const data  = ChildData::create();
    Child::Ptr           child = Child::create(shared_from_this<ProgressInterfaceMultiplexer>(), data);

    boost::mutex::scoped_lock const l(mut);
    children.insert(data);

    return child;
  }

  void ProgressInterfaceMultiplexer::unsubscribe(const ChildData::Ptr& data)
  {
    boost::mutex::scoped_lock const l(mut);
    children.erase(data);
  }

  void ProgressInterfaceMultiplexer::updateProgressState()
  {
    ProgressStateInterface::State state    = ProgressStateInterface::IDLE;
    double                        progress = 0.0;
    int                           workers  = 0;

    boost::mutex::scoped_lock const l(mut);
    for(const ChildData::Ptr& child: children)
    {
      boost::mutex::scoped_lock const child_lock(child->mut);
      switch(child->state)
      {
      case ProgressStateInterface::IDLE:
        break;
      case ProgressStateInterface::WAITING:
        if(state == ProgressStateInterface::IDLE || state == ProgressStateInterface::FINISHED)
        {
          state = ProgressStateInterface::WAITING;
        }
        progress += child->progress;
        workers++;
        break;
      case ProgressStateInterface::WORKING:
        if(state != ProgressStateInterface::WORKING)
        {
          state = ProgressStateInterface::WORKING;
        }
        progress += child->progress;
        workers++;
        break;
      case ProgressStateInterface::FINISHED:
        if(state == ProgressStateInterface::IDLE)
        {
          state = ProgressStateInterface::FINISHED;
        }
        progress += 1.0;
        workers++;
        break;
      }
    }

    parent->setProgress(state, progress / workers);

    if(state == ProgressStateInterface::FINISHED)
    {
      for(const ChildData::Ptr& child: children)
      {
        child->clearFinished();
      }
    }
  }

} // namespace Scroom::Utils
