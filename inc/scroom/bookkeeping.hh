/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/stuff.hh>
#include <scroom/utilities.hh>

namespace Scroom::Bookkeeping
{
  using Scroom::Utils::Stuff;
  using Scroom::Utils::StuffList;

  namespace Detail
  {
    class TokenImpl;
    class TokenAddition;

    template <typename V>
    class ValueType;

    template <typename V>
    class LValue;
  } // namespace Detail

  class Token : public boost::shared_ptr<Detail::TokenImpl>
  {
  public:
    explicit Token(const boost::shared_ptr<Detail::TokenImpl>& t);
    explicit Token(const boost::weak_ptr<Detail::TokenImpl>& t);
    Token();
    explicit Token(const Stuff& s);
    explicit Token(const StuffList& l);

    void add(const Stuff& s) const;
    void add(const StuffList& l) const;
    void merge(Token& rhs) const;
    void merge(StuffList& l) const;

  public:
    Detail::TokenAddition operator+(const Stuff& rhs) const;
    Token const&          operator+=(const Stuff& rhs) const;
  };

  using WeakToken = boost::weak_ptr<Detail::TokenImpl>;

  template <typename K, typename V>
  class MapBase
    : public virtual Scroom::Utils::Base
    , public boost::noncopyable
  {
  private:
    using MapType = typename std::map<K, boost::weak_ptr<Detail::ValueType<V>>>;

    MapType              map;
    mutable boost::mutex mut;

  public:
    Token             reserve(const K& k);
    Token             reReserve(const K& k);
    void              remove(const K& k);
    void              remove(const K& k, const WeakToken& t);
    Detail::LValue<V> at(const K& k);
    void              set(const K& k, const V& v);
    V                 get(const K& k);
    std::list<K>      keys() const;
    std::list<V>      values() const;
  };

  template <typename K, typename V>
  class Map : public MapBase<K, V>
  {
  public:
    using Ptr = boost::shared_ptr<Map<K, V>>;

  public:
    static Ptr create();
  };

  //    template<typename V>
  //    class Map<Token, V> : public MapBase<Token,V>
  //    {
  //    public:
  //      typedef boost::shared_ptr<Map<Token, V> > Ptr;
  //
  //    public:
  //      static Ptr create();
  //
  //    public:
  //      void addMe(const Token& k, const V& v);
  //      Token add(const V& v);
  //      Token add(const Token& k, const V& v);
  //    };
  //
  //    template<typename V>
  //    class Map<WeakToken, V> : public MapBase<WeakToken,V>
  //    {
  //    public:
  //      typedef boost::shared_ptr<Map<WeakToken, V> > Ptr;
  //
  //    public:
  //      static Ptr create();
  //
  //    public:
  //      void addMe(const WeakToken& k, const V& v);
  //      Token add(const V& v);
  //      Token add(const WeakToken& k, const V& v);
  //    };

} // namespace Scroom::Bookkeeping

#include <scroom/impl/bookkeepingimpl.hh>
