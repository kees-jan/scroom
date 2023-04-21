/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>
#include <memory>

#include <boost/noncopyable.hpp>

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

    template <typename K, typename V>
    struct MapType
    {
      using Type = typename std::map<K, std::weak_ptr<Detail::ValueType<V>>>;
    };

    template <typename K, typename V>
    struct MapType<std::weak_ptr<K>, V>
    {
      using Type = typename Scroom::Utils::WeakKeyMap<std::weak_ptr<K>, std::weak_ptr<Detail::ValueType<V>>>;
    };

  } // namespace Detail

  class Token : public std::shared_ptr<Detail::TokenImpl>
  {
  public:
    explicit Token(const std::shared_ptr<Detail::TokenImpl>& t);
    explicit Token(const std::weak_ptr<Detail::TokenImpl>& t);
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

  using WeakToken = std::weak_ptr<Detail::TokenImpl>;

  template <typename K, typename V>
  class MapBase
    : public virtual Scroom::Utils::Base
    , public boost::noncopyable
  {
  private:
    using MapType = typename Detail::MapType<K, V>::Type;

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
    using Ptr = std::shared_ptr<Map<K, V>>;

  public:
    static Ptr create();
  };

} // namespace Scroom::Bookkeeping

#include <scroom/impl/bookkeepingimpl.hh>
