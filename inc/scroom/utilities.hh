/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

#ifndef _UTILITIES_HH
#define _UTILITIES_HH

#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

namespace Scroom
{
  namespace Utils
  {
    /**
     * Base class that inherits from boost::enable_shared_from_this.
     *
     * In an inheritance hierarchy, you can inherit from boost::enable_shared_from_this
     * only once. Otherwise, you'll get two copies of the internal weak
     * pointer, and only one gets initialized.
     *
     * To work with this restriction, this class inherits from
     * boost::enable_shared_from_this, and provides methods to dynamic_cast
     * to whatever subtype you like.
     *
     * You should always inherit virtually from this class, to ensure that even
     * in the face of multiple inheritance, there always is only one copy of
     * Base.
     */
    class Base : public boost::enable_shared_from_this<Base>
    {
    public:
      virtual ~Base() {}

      /**
       * Calls shared_from_this() with a built-in dynamic cast, to
       * make it usable in subclasses.
       */
      template<typename R>
      boost::shared_ptr<R> shared_from_this();

      /**
       * Calls shared_from_this() with a built-in dynamic cast, to
       * make it usable in subclasses.
       */
      template<typename R>
      boost::shared_ptr<R const> shared_from_this() const;
    };

    template <typename R>
    boost::shared_ptr<R> Base::shared_from_this()
    {
      return boost::dynamic_pointer_cast<R, Base>(boost::enable_shared_from_this<Base>::shared_from_this());
    }

    template <typename R>
    boost::shared_ptr<R const> Base::shared_from_this() const
    {
      return boost::dynamic_pointer_cast<R const, Base const>(boost::enable_shared_from_this<Base>::shared_from_this());
    }

    class Counter
    {
    public:
      typedef boost::shared_ptr<Counter> Ptr;

      class Registrar
      {
      private:
        std::string name;
        Ptr counter;

      public:
        Registrar(std::string name, long& count);
        void ping() { /* counter->dump(); */ }
        ~Registrar();
      };

    private:
      std::map<std::string, long*> counts;
      boost::mutex mut;

    private:
      Counter();

    public:
      static Ptr create();
      void registerClass(std::string name, long& count);
      void unregisterClass(std::string name);
      void dump();
      std::map<std::string, long*> getCounts();
    };

    Counter::Ptr getCounter();
    void dumpCounts();

    template <class C>
    class Counted
    {
    private:
      static boost::mutex mut;
      static long count;
      static Counter::Registrar r;

    public:
      Counted()
      {
        boost::unique_lock<boost::mutex> lock(mut);
        count++;
        r.ping();
      }

      Counted(const Counted&)
      {
        boost::unique_lock<boost::mutex> lock(mut);
        count++;
        r.ping();
      }

      virtual ~Counted()
      {
        boost::unique_lock<boost::mutex> lock(mut);
        count--;
        r.ping();
      }
    };

    template <class C>
    boost::mutex Counted<C>::mut;
    
    template <class C>
    long Counted<C>::count = 0;

    template <class C>
    Counter::Registrar Counted<C>::r(typeid(C).name(), Counted<C>::count);

    }
}

#endif
