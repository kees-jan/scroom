/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>
#include <memory>
#include <utility>

#include <boost/thread.hpp>

#include <scroom/bookkeeping.hh>
#include <scroom/utilities.hh>

namespace Scroom::Utils
{
  template <typename T>
  class Observable;

  namespace Detail
  {
    /**
     * Contain all info regarding the registration.
     *
     * Performs de-registration on destruction
     */
    template <typename T>
    class Registration
    {
    public:
      std::weak_ptr<Observable<T>> observable;
      std::shared_ptr<T>           o;        /**< Reference to the observer (for non-weak registrations) */
      std::weak_ptr<T>             observer; /**< Reference to the observer */

      using Ptr = std::shared_ptr<Registration<T>>;

    public:
      Registration() = default;
      Registration(std::weak_ptr<Observable<T>> observable, std::shared_ptr<T> observer);
      Registration(std::weak_ptr<Observable<T>> observable, std::weak_ptr<T> observer);
      void set(std::shared_ptr<T> observer);
      void set(std::weak_ptr<T> observer);

      static Ptr create(std::weak_ptr<Observable<T>> observable, std::shared_ptr<T> observer);
      static Ptr create(std::weak_ptr<Observable<T>> observable, std::weak_ptr<T> observer);
    };
  } // namespace Detail

  /**
   * Base class for something that accepts observers.
   *
   * Upon registering, observers are given a Registration. If the
   * same Observer registers twice (using the same method), he'll
   * still receive events only once. When the Observer destroys his
   * Registration, he'll be unregistered.
   *
   * There are two ways of registering: One can register a @c
   * shared_ptr, or a @c weak_ptr, depending on whether you want the
   * Observable to keep the Observer from being destructed.
   *
   * @note When you register one Observer with both a @c shared_ptr
   *    and a @c weak_ptr, unpredictable things will happen.
   */
  template <typename T>
  class Observable : public virtual Base
  {
  public:
    using Observer = std::shared_ptr<T>;
    using Ptr      = std::shared_ptr<Observable<T>>;

  private:
    using ObserverWeak = std::weak_ptr<T>;
    using Registration = Detail::Registration<T>;

  private:
    /** Map all registrations to their registration data */
    typename Scroom::Bookkeeping::Map<ObserverWeak, typename Registration::Ptr>::Ptr registrationMap;

  protected:
    /**
     * Retrieve a list of current observers.
     *
     * Always, a list of @c shared_ptr objects is returned,
     * regardless of whether the Observer registered a @c shared_ptr
     * or a @c weak_ptr.
     */
    std::list<Observer> getObservers();

  public:
    Observable();
    ~Observable() override;
    Observable(const Observable&)           = delete;
    Observable(Observable&&)                = delete;
    Observable operator=(const Observable&) = delete;
    Observable operator=(Observable&&)      = delete;

  protected:
    /**
     * Override this function if you want to be notified of new
     * observers registering.
     */
    virtual void observerAdded(Observer const& observer, Scroom::Bookkeeping::Token const& token);

  public:
    Scroom::Bookkeeping::Token registerStrongObserver(Observer const& observer);
    Scroom::Bookkeeping::Token registerObserver(ObserverWeak const& observer);

  private:
    void unregisterObserver(ObserverWeak const& observer);

    friend class Detail::Registration<T>;
  };

  ////////////////////////////////////////////////////////////////////////
  // Detail::Registration implementation

  template <typename T>
  Detail::Registration<T>::Registration(std::weak_ptr<Observable<T>> observable_, std::shared_ptr<T> observer_)
    : observable(std::move(observable_))
    , o(observer_)
    , observer(observer_)
  {
  }

  template <typename T>
  Detail::Registration<T>::Registration(std::weak_ptr<Observable<T>> observable_, std::weak_ptr<T> observer_)
    : observable(std::move(observable_))
    , o()
    , observer(std::move(observer_))
  {
    // We don't want to store a "hard" reference, so field o is intentionally empty.
  }

  template <typename T>
  void Detail::Registration<T>::set(std::shared_ptr<T> observer_)
  {
    o        = observer_;
    observer = observer_;
  }

  template <typename T>
  void Detail::Registration<T>::set(std::weak_ptr<T> observer_)
  {
    // We don't want to store a "hard" reference, so field o is intentionally empty.
    o.reset();
    observer = observer_;
  }

  template <typename T>
  typename Detail::Registration<T>::Ptr Detail::Registration<T>::create(std::weak_ptr<Observable<T>> observable,
                                                                        std::shared_ptr<T>           observer)
  {
    return typename Detail::Registration<T>::Ptr(new Detail::Registration<T>(observable, observer));
  }

  template <typename T>
  typename Detail::Registration<T>::Ptr Detail::Registration<T>::create(std::weak_ptr<Observable<T>> observable,
                                                                        std::weak_ptr<T>             observer)
  {
    return typename Detail::Registration<T>::Ptr(new Detail::Registration<T>(observable, observer));
  }

  ////////////////////////////////////////////////////////////////////////
  // Observable implementation
  template <typename T>
  Observable<T>::Observable()
  {
    registrationMap = Scroom::Bookkeeping::Map<ObserverWeak, typename Registration::Ptr>::create();
  }

  template <typename T>
  Observable<T>::~Observable()
  {
    // Destroy strong references to any observers
    //
    // The one holding the token of the registration is in control
    // of the lifetime of the registration objects. Hence, as long
    // as the Token is valid, a reference to the Registration will
    // exist, which may, in turn, contain a (strong) reference to an
    // Observer. This is not desirable. As soon as the observable is
    // deleted (which is now), it should no longer hold any
    // references to any Observers. Hence, we should manually reset
    // references to observers here.
    for(const typename Registration::Ptr& registration: registrationMap->values())
    {
      registration->o.reset();
    }
  }

  template <typename T>
  std::list<typename Observable<T>::Observer> Observable<T>::getObservers()
  {
    std::list<typename Observable<T>::Observer> result;
    for(const typename Registration::Ptr& registration: registrationMap->values())
    {
      Observer const o = registration->observer.lock();
      if(o)
      {
        result.push_back(o);
      }
    }

    return result;
  }

  template <typename T>
  Scroom::Bookkeeping::Token Observable<T>::registerStrongObserver(Observable<T>::Observer const& observer)
  {
    Scroom::Bookkeeping::Token            t = registrationMap->reReserve(observer);
    typename Detail::Registration<T>::Ptr r = registrationMap->get(observer);
    if(r)
    {
      r->set(observer);
    }
    else
    {
      r = Detail::Registration<T>::create(shared_from_this<Observable<T>>(), observer);
      registrationMap->set(observer, r);
    }

    observerAdded(observer, t);

    return t;
  }

  template <typename T>
  Scroom::Bookkeeping::Token Observable<T>::registerObserver(Observable<T>::ObserverWeak const& observer)
  {
    Scroom::Bookkeeping::Token            t = registrationMap->reReserve(observer);
    typename Detail::Registration<T>::Ptr r = registrationMap->get(observer);
    if(r)
    {
      r->set(observer);
    }
    else
    {
      r = Detail::Registration<T>::create(shared_from_this<Observable<T>>(), observer);
      registrationMap->set(observer, r);
    }

    observerAdded(typename Observable<T>::Observer(observer), t);

    return t;
  }

  template <typename T>
  void Observable<T>::unregisterObserver(Observable<T>::ObserverWeak const& observer)
  {
    registrationMap->remove(observer);
  }

  template <typename T>
  void Observable<T>::observerAdded(Observable<T>::Observer const& /*unused*/, Scroom::Bookkeeping::Token const& /*unused*/)
  {
    // Do nothing
  }
} // namespace Scroom::Utils
