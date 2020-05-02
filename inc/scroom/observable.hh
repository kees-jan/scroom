/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/utilities.hh>
#include <scroom/bookkeeping.hh>

namespace Scroom
{
  namespace Utils
  {
    template<typename T>
    class Observable;

    namespace Detail
    {
      /**
       * Contain all info regarding the registration.
       *
       * Performs de-registration on destruction
       */
      template<typename T>
      class Registration
      {
      public:
        boost::weak_ptr<Observable<T> > observable;
        boost::shared_ptr<T> o;      /**< Reference to the observer (for non-weak registrations) */
        boost::weak_ptr<T> observer; /**< Reference to the observer */

        typedef boost::shared_ptr<Registration> Ptr;

      public:
        Registration();
        Registration(boost::weak_ptr<Observable<T> > observable, boost::shared_ptr<T> observer);
        Registration(boost::weak_ptr<Observable<T> > observable, boost::weak_ptr<T> observer);
        ~Registration();
        void set(boost::shared_ptr<T> observer);
        void set(boost::weak_ptr<T> observer);

        static Ptr create(boost::weak_ptr<Observable<T> > observable, boost::shared_ptr<T> observer);
        static Ptr create(boost::weak_ptr<Observable<T> > observable, boost::weak_ptr<T> observer);
      };
    }

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
    template<typename T>
    class Observable : public boost::noncopyable,
                       public virtual Base
    {
    public:
      typedef boost::shared_ptr<T> Observer;
      typedef boost::shared_ptr<Observable<T> > Ptr;

    private:
      typedef boost::weak_ptr<T> ObserverWeak;
      typedef Detail::Registration<T> Registration;

    private:
      /** Map all registrations to their registration data */
      typename Scroom::Bookkeeping::Map<ObserverWeak, typename Registration::Ptr >::Ptr registrationMap;

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
      virtual ~Observable();

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

    template<typename T>
    Detail::Registration<T>::Registration()
      : observable(), o(), observer()
    {
    }

    template<typename T>
    Detail::Registration<T>::Registration(boost::weak_ptr<Observable<T> > observable_, boost::shared_ptr<T> observer_)
      : observable(observable_), o(observer_), observer(observer_)
    {
    }

    template<typename T>
    Detail::Registration<T>::Registration(boost::weak_ptr<Observable<T> > observable_, boost::weak_ptr<T> observer_)
      : observable(observable_), o(), observer(observer_)
    {
      // We don't want to store a "hard" reference, so field o is intentionally empty.
    }

    template<typename T>
    void Detail::Registration<T>::set(boost::shared_ptr<T> observer_)
    {
      this->o = observer_;
      this->observer = observer_;
    }

    template<typename T>
    void Detail::Registration<T>::set(boost::weak_ptr<T> observer_)
    {
      // We don't want to store a "hard" reference, so field o is intentionally empty.
      this->o.reset();
      this->observer = observer_;
    }

    template<typename T>
    typename Detail::Registration<T>::Ptr Detail::Registration<T>::create(boost::weak_ptr<Observable<T> > observable,
                                                           boost::shared_ptr<T> observer)
    {
      return typename Detail::Registration<T>::Ptr(new Detail::Registration<T>(observable, observer));
    }

    template<typename T>
    typename Detail::Registration<T>::Ptr Detail::Registration<T>::create(boost::weak_ptr<Observable<T> > observable,
                                                           boost::weak_ptr<T> observer)
    {
      return typename Detail::Registration<T>::Ptr(new Detail::Registration<T>(observable, observer));
    }

    template<typename T>
    Detail::Registration<T>::~Registration()
    {
    }

    ////////////////////////////////////////////////////////////////////////
    // Observable implementation
    template<typename T>
    Observable<T>::Observable()
    {
      registrationMap = Scroom::Bookkeeping::Map<ObserverWeak, typename Registration::Ptr >::create();
    }

    template<typename T>
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
      for(typename Registration::Ptr registration: registrationMap->values())
      {
        registration->o.reset();
      }
    }

    template<typename T>
    std::list<typename Observable<T>::Observer> Observable<T>::getObservers()
    {
      std::list<typename Observable<T>::Observer> result;
      for(typename Registration::Ptr registration: registrationMap->values())
      {
        Observer o = registration->observer.lock();
        if(o)
          result.push_back(o);
      }

      return result;
    }

    template<typename T>
    Scroom::Bookkeeping::Token Observable<T>::registerStrongObserver(Observable<T>::Observer const& observer)
    {
      Scroom::Bookkeeping::Token t = registrationMap->reReserve(observer);
      typename Detail::Registration<T>::Ptr r = registrationMap->get(observer);
      if(r)
      {
        r->set(observer);
      }
      else
      {
        r = Detail::Registration<T>::create(shared_from_this<Observable<T> >(), observer);
        registrationMap->set(observer, r);
      }

      observerAdded(observer, t);

      return t;
    }

    template<typename T>
    Scroom::Bookkeeping::Token Observable<T>::registerObserver(Observable<T>::ObserverWeak const& observer)
    {
      Scroom::Bookkeeping::Token t = registrationMap->reReserve(observer);
      typename Detail::Registration<T>::Ptr r = registrationMap->get(observer);
      if(r)
      {
        r->set(observer);
      }
      else
      {
        r = Detail::Registration<T>::create(shared_from_this<Observable<T> >(), observer);
        registrationMap->set(observer,r);
      }

      observerAdded(typename Observable<T>::Observer(observer), t);

      return t;
    }

    template<typename T>
    void Observable<T>::unregisterObserver(Observable<T>::ObserverWeak const& observer)
    {
      registrationMap->remove(observer);
    }

    template<typename T>
    void Observable<T>::observerAdded(Observable<T>::Observer const&, Scroom::Bookkeeping::Token const&)
    {
      // Do nothing
    }
  }
}

