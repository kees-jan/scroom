/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <map>
#include <memory>
#include <set>

#include <boost/thread.hpp>

namespace Scroom::Utils
{
  /**
   * Base class that inherits from std::enable_shared_from_this.
   *
   * In an inheritance hierarchy, you can inherit from std::enable_shared_from_this
   * only once. Otherwise, you'll get two copies of the internal weak
   * pointer, and only one gets initialized.
   *
   * To work with this restriction, this class inherits from
   * std::enable_shared_from_this, and provides methods to dynamic_cast
   * to whatever subtype you like.
   *
   * You should always inherit virtually from this class, to ensure that even
   * in the face of multiple inheritance, there always is only one copy of
   * Base.
   */
  class Base : public std::enable_shared_from_this<Base>
  {
  public:
    Base()                       = default;
    Base(const Base&)            = delete;
    Base(Base&&)                 = delete;
    Base& operator=(const Base&) = delete;
    Base& operator=(Base&&)      = delete;
    virtual ~Base()              = default;

    /**
     * Calls shared_from_this() with a built-in dynamic cast, to
     * make it usable in subclasses.
     */
    template <typename R>
    std::shared_ptr<R> shared_from_this()
    {
      return std::dynamic_pointer_cast<R, Base>(std::enable_shared_from_this<Base>::shared_from_this());
    }

    /**
     * Calls shared_from_this() with a built-in dynamic cast, to
     * make it usable in subclasses.
     */
    template <typename R>
    std::shared_ptr<R const> shared_from_this() const
    {
      return std::dynamic_pointer_cast<R const, Base const>(std::enable_shared_from_this<Base>::shared_from_this());
    }
  };

  template <typename F>
  class on_scope_exit
  {
  public:
    explicit on_scope_exit(F f_)
      : f(std::move(f_))
    {
    }
    on_scope_exit(const on_scope_exit&)            = delete;
    on_scope_exit(on_scope_exit&&)                 = delete;
    on_scope_exit& operator=(const on_scope_exit&) = delete;
    on_scope_exit& operator=(on_scope_exit&&)      = delete;

    ~on_scope_exit() { f(); }

  private:
    F f;
  };

  template <typename F>
  std::shared_ptr<void> on_destruction(F f)
  {
    return std::make_shared<on_scope_exit<F>>(std::move(f));
  }

  template <typename F>
  class optional_cleanup
  {
  public:
    explicit optional_cleanup(F f_)
      : f(std::move(f_))
    {
    }
    optional_cleanup(const optional_cleanup&)            = delete;
    optional_cleanup(optional_cleanup&&)                 = delete;
    optional_cleanup& operator=(const optional_cleanup&) = delete;
    optional_cleanup& operator=(optional_cleanup&&)      = delete;

    ~optional_cleanup()
    {
      if(cleanup)
      {
        f();
      }
    }

    void cancel() { cleanup = false; }

  private:
    bool cleanup{true};
    F    f;
  };

  template <typename K, typename V>
  using WeakKeyMap = std::map<K, V, std::owner_less<K>>;

  template <typename K>
  using WeakKeySet = std::set<K, std::owner_less<K>>;

  inline std::shared_ptr<unsigned char> shared_malloc(size_t size) { return {static_cast<unsigned char*>(malloc(size)), free}; }

  ////////////////////////////////////////////////////////////////////////

  void dumpCounts();

  ////////////////////////////////////////////////////////////////////////

  class Count
  {
  public:
    using Ptr = std::shared_ptr<Count>;

  public:
    const std::string name;
    boost::mutex      mut;
    long              count{0};

  public:
    static Ptr create(const std::string& name);
    void       ping()
    { /* dumpCounts(); */
    }
    void inc()
    {
      boost::unique_lock<boost::mutex> const lock(mut);
      ++count;
      ping();
    }
    void dec()
    {
      boost::unique_lock<boost::mutex> const lock(mut);
      --count;
      ping();
    }

  private:
    explicit Count(std::string name);
  };

  ////////////////////////////////////////////////////////////////////////

  class Counter
  {
  public:
  private:
    std::list<Count::Ptr> counts;
    boost::mutex          mut;

  private:
    Counter();

  public:
    static Counter*       instance();
    void                  registerCount(const Count::Ptr& count);
    void                  unregisterCount(const Count::Ptr& count);
    void                  dump();
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
      static Count::Ptr const instance = Count::create(typeid(C).name());
      return instance;
    }

    Counted()
      : data(count_instance())
    {
      data->inc();
    }

    Counted(const Counted& /*unused*/)
      : data(count_instance())
    {
      data->inc();
    }

    Counted(Counted&& /*unused*/)
      : data(count_instance())
    {
      data->inc();
    }

    Counted& operator=(const Counted&) = default;
    Counted& operator=(Counted&&)      = default;

    virtual ~Counted() { data->dec(); }
  };
} // namespace Scroom::Utils
