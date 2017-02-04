/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

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

    ////////////////////////////////////////////////////////////////////////

    void dumpCounts();

    ////////////////////////////////////////////////////////////////////////

    class Count
    {
    public:
      typedef boost::shared_ptr<Count> Ptr;
      
    public:
      const std::string name;
      boost::mutex mut;
      long count;

    public:
      static Ptr create(const std::string& name);
      void ping() { /* dumpCounts(); */ }
      void inc()
      { boost::unique_lock<boost::mutex> lock(mut); ++count; ping(); }
      void dec()
      { boost::unique_lock<boost::mutex> lock(mut); --count; ping(); }

    private:
      Count(const std::string& name);
    };

    ////////////////////////////////////////////////////////////////////////
    
    class Counter
    {
    public:

    private:
      std::list<Count::Ptr> counts;
      boost::mutex mut;

    private:
      Counter();

    public:
      static Counter* instance();
      void registerCount(Count::Ptr count);
      void unregisterCount(Count::Ptr count);
      void dump();
      std::list<Count::Ptr> getCounts();
    };

    ////////////////////////////////////////////////////////////////////////

    template <class C>
    class Counted
    {
    private:
      Count::Ptr data;

    public:
      static Count::Ptr count_instance()
      {
        static Count::Ptr instance = Count::create(typeid(C).name());
        return instance;
      }
    
      Counted()
        : data(count_instance())
      {
        data->inc();
      }

      Counted(const Counted&)
        : data(count_instance())
      {
        data->inc();
      }

      virtual ~Counted()
      {
        data->dec();
      }
    };
  }
}


