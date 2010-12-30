/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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
#include <boost/enable_shared_from_this.hpp>

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
      struct Registration
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

        static Ptr create(boost::weak_ptr<Observable<T> > observable, boost::shared_ptr<T> observer);
        static Ptr create(boost::weak_ptr<Observable<T> > observable, boost::weak_ptr<T> observer);
      };
    }

    /** A Registration is a pointer to some private data. */
    typedef boost::shared_ptr<void> Registration;

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
                       public boost::enable_shared_from_this<Observable<T> >
    {
    public:
      typedef boost::shared_ptr<T> Observer;

    private:
      typedef boost::weak_ptr<Detail::Registration<T> > RegistrationWeak;
      typedef boost::weak_ptr<T> ObserverWeak;
      typedef std::map<Observer, RegistrationWeak> RegistrationMap;
      typedef std::map<ObserverWeak, RegistrationWeak> RegistrationMapWeak;
      
    private:
      /** Map all @c shared_ptr registrations to their registration data */
      RegistrationMap registrationMap;

      /** Map all @c weak_ptr registrations to their registration data */
      RegistrationMapWeak registrationMapWeak;

      /** Mutex protecting the two maps */
      boost::mutex mut;
  
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

    public:
      Registration registerObserver(Observer observer);
      Registration registerWeakObserver(Observer observer);
      Registration registerObserver(ObserverWeak observer);

    private:
      void unregisterObserver(Observer observer);
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
    typename Detail::Registration<T>::Ptr Detail::Registration<T>::create(boost::weak_ptr<Observable<T> > observable,
                                                           boost::shared_ptr<T> observer)
    {
      return Detail::Registration<T>::Ptr(new Detail::Registration<T>(observable, observer));
    }
        
    template<typename T>
    typename Detail::Registration<T>::Ptr Detail::Registration<T>::create(boost::weak_ptr<Observable<T> > observable,
                                                           boost::weak_ptr<T> observer)
    {
      return Detail::Registration<T>::Ptr(new Detail::Registration<T>(observable, observer));
    }
    
    template<typename T>
    Detail::Registration<T>::~Registration()
    {
      boost::shared_ptr<Observable<T> > observable = this->observable.lock();
      if(observable)
      {
        if(o)
        {
          observable->unregisterObserver(o);
        }
        else
        {
          observable->unregisterObserver(observer);
        }
      }
    }
    
    ////////////////////////////////////////////////////////////////////////
    // Observable implementation
    template<typename T>
    Observable<T>::Observable()
    {
    }

    template<typename T>
    std::list<typename Observable<T>::Observer> Observable<T>::getObservers()
    {
      boost::mutex::scoped_lock lock(mut);
      std::list<Observable<T>::Observer> result;

      {
        typename RegistrationMap::iterator cur = registrationMap.begin();
        typename RegistrationMap::iterator end = registrationMap.end();

        for(;cur!=end; ++cur)
        {
          typename Detail::Registration<T>::Ptr registration = cur->second.lock();
          if(registration)
            result.push_back(registration->o);
        }
      }
      {
        typename RegistrationMapWeak::iterator cur = registrationMapWeak.begin();
        typename RegistrationMapWeak::iterator end = registrationMapWeak.end();

        for(;cur!=end; ++cur)
        {
          typename Detail::Registration<T>::Ptr registration = cur->second.lock();
          if(registration)
          {
            Observer o = registration->observer.lock();
            if(o)
              result.push_back(o);
          }
        }
      }

      return result;
    }

    template<typename T>
    Registration Observable<T>::registerObserver(Observable<T>::Observer observer)
    {
      boost::mutex::scoped_lock lock(mut);
      typename Detail::Registration<T>::Ptr r = registrationMap[observer].lock();
      if(!r)
      {
        r = Detail::Registration<T>::create(this->shared_from_this(), observer);
        registrationMap[observer] = r;
      }
      return r;
    }

    template<typename T>
    Registration Observable<T>::registerWeakObserver(Observable<T>::Observer observer)
    {
      Observable<T>::ObserverWeak observerWeak = observer;
      
      return registerObserver(observerWeak);
    }

    template<typename T>
    Registration Observable<T>::registerObserver(Observable<T>::ObserverWeak observer)
    {
      boost::mutex::scoped_lock lock(mut);
      typename Detail::Registration<T>::Ptr r = registrationMapWeak[observer].lock();
      if(!r)
      {
        r = Detail::Registration<T>::create(this->shared_from_this(), observer);
        registrationMapWeak[observer] = r;
      }
        
      return r;
    }

    template<typename T>
    void Observable<T>::unregisterObserver(Observable<T>::Observer observer)
    {
      boost::mutex::scoped_lock lock(mut);
      // observers.remove(observer);
    }

    template<typename T>
    void Observable<T>::unregisterObserver(Observable<T>::ObserverWeak observer)
    {
      boost::mutex::scoped_lock lock(mut);
      // observers.remove(observer);
    }
  }
}

#endif
