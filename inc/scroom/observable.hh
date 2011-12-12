/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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

#ifndef OBSERVABLE_HH
#define OBSERVABLE_HH

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
        StuffList registrations; /**< Recursive registrations */

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

      /**
       * Override this function if you want to be notified of new
       * observers registering.
       */
      virtual void observerAdded(Observer observer);

      /**
       * Add the given @c registration to the Detail::Registration of
       * the given @c observer, such that they have the same lifetime.
       *
       * You'll typically use this if you want to register an Observer
       * with a bunch of child observables. Registering with child
       * observables will result in you being given a bunch of
       * Registration objects. This is where you store them :-)
       *
       * @pre @c observer's Registration must exist.
       *
       * @throw boost::bad_weak_ptr if the registration doesn't exist
       */
      void addRecursiveRegistration(Observer observer, Stuff registration);

    public:
      Stuff registerStrongObserver(Observer observer);
      Stuff registerObserver(ObserverWeak observer);

    private:
      void unregisterObserver(ObserverWeak observer);

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
    Detail::Registration<T>::Registration(boost::weak_ptr<Observable<T> > observable, boost::shared_ptr<T> observer)
      : observable(observable), o(observer), observer(observer)
    {
    }
    
    template<typename T>
    Detail::Registration<T>::Registration(boost::weak_ptr<Observable<T> > observable, boost::weak_ptr<T> observer)
      : observable(observable), o(), observer(observer)
    {
      // We don't want to store a "hard" reference, so field o is intentionally empty.
    }

    template<typename T>
    void Detail::Registration<T>::set(boost::shared_ptr<T> observer)
    {
      this->o = observer;
      this->observer = observer;
    }
    
    template<typename T>
    void Detail::Registration<T>::set(boost::weak_ptr<T> observer)
    {
      // We don't want to store a "hard" reference, so field o is intentionally empty.
      this->o.reset();
      this->observer = observer;
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
      BOOST_FOREACH(typename Registration::Ptr registration, registrationMap->values())
      {
        registration->o.reset();
      }
    }

    template<typename T>
    std::list<typename Observable<T>::Observer> Observable<T>::getObservers()
    {
      std::list<typename Observable<T>::Observer> result;
      BOOST_FOREACH(typename Registration::Ptr registration, registrationMap->values())
      {
        Observer o = registration->observer.lock();
        if(o)
          result.push_back(o);
      }

      return result;
    }

    template<typename T>
    Stuff Observable<T>::registerStrongObserver(Observable<T>::Observer observer)
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

      observerAdded(observer);

      return t;
    }

    template<typename T>
    Stuff Observable<T>::registerObserver(Observable<T>::ObserverWeak observer)
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

      observerAdded(typename Observable<T>::Observer(observer));
        
      return t;
    }

    template<typename T>
    void Observable<T>::unregisterObserver(Observable<T>::ObserverWeak observer)
    {
      registrationMap->remove(observer);
    }

    template<typename T>
    void Observable<T>::observerAdded(Observable<T>::Observer)
    {
      // Do nothing
    }

    template<typename T>
    void Observable<T>::addRecursiveRegistration(Observable<T>::Observer observer, Stuff registration)
    {
      registrationMap->get(observer)->registrations.push_back(registration);
    }
  }
}

#endif
