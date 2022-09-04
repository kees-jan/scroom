/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <stdexcept>
#include <utility>

namespace Scroom::Bookkeeping
{
  namespace Detail
  {
    class TokenAddition : public Scroom::Bookkeeping::Token
    {
    public:
      explicit TokenAddition(const Scroom::Bookkeeping::Token& t)
        : Scroom::Bookkeeping::Token(t)
      {
      }

      TokenAddition& operator+(const Stuff& rhs)
      {
        add(rhs);
        return *this;
      }

      TokenAddition& operator+=(const Stuff& rhs)
      {
        add(rhs);
        return *this;
      }

      TokenAddition& operator+(TokenAddition& rhs)
      {
        merge(rhs);
        return *this;
      }

      TokenAddition& operator+=(TokenAddition& rhs)
      {
        merge(rhs);
        return *this;
      }
    };

    class TokenImpl
    {
    public:
      using Ptr = boost::shared_ptr<TokenImpl>;

    public:
      void add(const Stuff& s) { l.push_back(s); }

      void add(const StuffList& l_) { l.insert(l.end(), l_.begin(), l_.end()); }

      void merge(StuffList& l_) { l.splice(l.end(), l_); }

      void merge(Ptr& rhs) { merge(rhs->l); }

    public:
      static Scroom::Bookkeeping::Token create() { return Scroom::Bookkeeping::Token(Ptr(new TokenImpl)); }

    protected:
      TokenImpl() = default;

    private:
      StuffList l;
    };

    template <typename K, typename V>
    class MapTokenImpl : public TokenImpl
    {
    public:
      using Ptr = boost::shared_ptr<MapTokenImpl<K, V>>;

    private:
      boost::weak_ptr<Scroom::Bookkeeping::MapBase<K, V>> map;
      WeakToken                                           t;
      K                                                   k;

    protected:
      MapTokenImpl(boost::shared_ptr<Scroom::Bookkeeping::MapBase<K, V>> map_, K k_)
        : map(map_)
        , k(std::move(k_))
      {
      }

      MapTokenImpl(const MapTokenImpl&)           = delete;
      MapTokenImpl(MapTokenImpl&&)                = delete;
      MapTokenImpl operator=(const MapTokenImpl&) = delete;
      MapTokenImpl operator=(MapTokenImpl&&)      = delete;

    public:
      ~MapTokenImpl()
      {
        boost::shared_ptr<Scroom::Bookkeeping::MapBase<K, V>> const m = map.lock();
        if(m)
        {
          m->remove(k, t);
        }
      }

    public:
      static Scroom::Bookkeeping::Token create(boost::shared_ptr<Scroom::Bookkeeping::MapBase<K, V>> map, const K& k)
      {
        Ptr const t = Ptr(new MapTokenImpl<K, V>(map, k));
        t->t        = t;
        return Scroom::Bookkeeping::Token(TokenImpl::Ptr(t));
      }
    };

    template <typename V>
    class ValueType
    {
    public:
      using Ptr     = boost::shared_ptr<ValueType<V>>;
      using WeakPtr = boost::weak_ptr<ValueType<V>>;

    public:
      V         value;
      WeakToken token;

    protected:
      explicit ValueType(V value_)
        : value(std::move(value_))
      {
      }

    public:
      static Ptr create(V value) { return Ptr(new ValueType<V>(value)); }
    };

    template <typename V>
    class LValue
    {
    public:
      using VTPtr = typename ValueType<V>::Ptr;

    private:
      VTPtr pv;

    public:
      explicit LValue(VTPtr pv_)
        : pv(std::move(pv_))
      {
      }

      LValue& operator=(const V& v)
      {
        pv->value = v;
        return *this;
      }

      explicit operator V() { return pv->value; }
    };
  } // namespace Detail

  ////////////////////////////////////////////////////////////////////////

  inline Token::Token(const boost::shared_ptr<Detail::TokenImpl>& t)
    : boost::shared_ptr<Detail::TokenImpl>(t)
  {
  }

  inline Token::Token(const boost::weak_ptr<Detail::TokenImpl>& t)
    : boost::shared_ptr<Detail::TokenImpl>(t)
  {
  }

  inline Token::Token()
    : boost::shared_ptr<Detail::TokenImpl>(Detail::TokenImpl::create())
  {
  }

  inline Token::Token(const Stuff& s)
    : boost::shared_ptr<Detail::TokenImpl>(Detail::TokenImpl::create())
  {
    get()->add(s);
  }

  inline Token::Token(const StuffList& l)
    : boost::shared_ptr<Detail::TokenImpl>(Detail::TokenImpl::create())
  {
    get()->add(l);
  }

  inline void Token::add(const Stuff& s) const { get()->add(s); }

  inline void Token::add(const StuffList& l) const { get()->add(l); }

  inline void Token::merge(Token& rhs) const { get()->merge(rhs); }

  inline void Token::merge(StuffList& l) const { get()->merge(l); }

  inline Detail::TokenAddition Token::operator+(const Stuff& rhs) const { return Detail::TokenAddition(*this) + rhs; }

  inline Token const& Token::operator+=(const Stuff& rhs) const
  {
    add(rhs);
    return *this;
  }

  ////////////////////////////////////////////////////////////////////////

  template <typename K, typename V>
  inline Token MapBase<K, V>::reserve(const K& k)
  {
    boost::mutex::scoped_lock const lock(mut);
    if(map.end() != map.find(k))
    {
      throw std::invalid_argument("Key already exists");
    }

    typename Detail::ValueType<V>::Ptr const pv = Detail::ValueType<V>::create(V());
    map[k]                                      = pv;

    Token t = Detail::MapTokenImpl<K, V>::create(shared_from_this<MapBase<K, V>>(), k);
    t.add(pv);
    pv->token = t;
    return t;
  }

  template <typename K, typename V>
  inline Token MapBase<K, V>::reReserve(const K& k)
  {
    boost::mutex::scoped_lock const lock(mut);
    auto                            i = map.find(k);

    if(map.end() == i)
    {
      map[k] = typename Detail::ValueType<V>::WeakPtr();
      i      = map.find(k);
    }

    typename Detail::ValueType<V>::Ptr pv = i->second.lock();
    if(!pv)
    {
      pv        = Detail::ValueType<V>::create(V());
      i->second = pv;
    }

    Token t(pv->token.lock());
    if(!t)
    {
      t = Detail::MapTokenImpl<K, V>::create(shared_from_this<MapBase<K, V>>(), k);
      t.add(pv);
      pv->token = t;
    }

    return t;
  }

  template <typename K, typename V>
  inline void MapBase<K, V>::remove(const K& k, const WeakToken& wt)
  {
    boost::mutex::scoped_lock const lock(mut);
    auto                            i = map.find(k);

    if(map.end() != i)
    {
      typename Detail::ValueType<V>::Ptr const pv = i->second.lock();
      if(pv)
      {
        Token const t(wt.lock());
        Token const t_orig(pv->token.lock());
        if(t == t_orig)
        {
          map.erase(i);
        }
      }
      else
      {
        map.erase(i);
      }
    }
  }

  template <typename K, typename V>
  inline void MapBase<K, V>::remove(const K& k)
  {
    boost::mutex::scoped_lock  lock(mut);
    typename MapType::iterator i = map.find(k);

    if(map.end() != i)
    {
      map.erase(i);
    }
  }

  template <typename K, typename V>
  inline Detail::LValue<V> MapBase<K, V>::at(const K& k)
  {
    boost::mutex::scoped_lock const lock(mut);
    auto                            i = map.find(k);

    if(map.end() != i)
    {
      typename Detail::ValueType<V>::Ptr const pv = i->second.lock();
      if(pv)
      {
        return Detail::LValue<V>(pv);
      }
    }

    throw std::invalid_argument("Invalid key");
  }

  template <typename K, typename V>
  inline void MapBase<K, V>::set(const K& k, const V& v)
  {
    boost::mutex::scoped_lock const lock(mut);
    auto                            i = map.find(k);

    if(map.end() != i)
    {
      typename Detail::ValueType<V>::Ptr const pv = i->second.lock();
      if(pv)
      {
        pv->value = v;
        return;
      }
    }

    throw std::invalid_argument("Invalid key");
  }

  template <typename K, typename V>
  inline V MapBase<K, V>::get(const K& k)
  {
    boost::mutex::scoped_lock const lock(mut);
    auto                            i = map.find(k);

    if(map.end() != i)
    {
      typename Detail::ValueType<V>::Ptr const pv = i->second.lock();
      if(pv)
      {
        return pv->value;
      }
    }

    throw std::invalid_argument("Invalid key");
  }

  template <typename K, typename V>
  inline std::list<K> MapBase<K, V>::keys() const
  {
    boost::mutex::scoped_lock const lock(mut);
    std::list<K>                    result;
    for(const typename MapType::value_type& el: map)
    {
      result.push_back(el.first);
    }
    return result;
  }

  template <typename K, typename V>
  inline std::list<V> MapBase<K, V>::values() const
  {
    boost::mutex::scoped_lock const lock(mut);
    std::list<V>                    result;
    for(const typename MapType::value_type& el: map)
    {
      typename Detail::ValueType<V>::Ptr const pv = el.second.lock();
      if(pv)
      {
        result.push_back(pv->value);
      }
    }
    return result;
  }

  ////////////////////////////////////////////////////////////////////////

  //    template<typename V>
  //    inline void Map<Token, V>::addMe(const Token& k, const V& v)
  //    {
  //      k->add(add(k,v));
  //    }
  //
  //    template<typename V>
  //    inline void Map<WeakToken, V>::addMe(const WeakToken& k, const V& v)
  //    {
  //      Token K = k.lock();
  //      if(K)
  //        K->add(add(k,v));
  //      else
  //        throw std::invalid_argument("boost::weak_ptr can't be locked");
  //    }
  //
  //    template<typename V>
  //    inline Token Map<Token, V>::add(const V& v)
  //    {
  //      Token k;
  //      k->add(add(k,v));
  //      return k;
  //    }
  //
  //    template<typename V>
  //    inline Token Map<WeakToken, V>::add(const V& v)
  //    {
  //      Token k;
  //      k->add(add(k,v));
  //      return k;
  //    }
  //
  //    template<typename V>
  //    inline Token Map<WeakToken, V>::add(const WeakToken& k, const V& v)
  //    {
  //      return MapBase<WeakToken, V>::add(k,v);
  //    }
  //
  //    template<typename V>
  //    inline Token Map<Token, V>::add(const Token& k, const V& v)
  //    {
  //      return MapBase<Token, V>::add(k,v);
  //    }

  template <typename K, typename V>
  inline typename Map<K, V>::Ptr Map<K, V>::create()
  {
    return Ptr(new Map<K, V>());
  }

  //     template<typename V>
  //     inline typename Map<Token, V>::Ptr Map<Token, V>::create()
  //     {
  //       return Ptr(new Map<Token, V>());
  //     }
  //
  //     template<typename V>
  //     inline typename Map<WeakToken, V>::Ptr Map<WeakToken, V>::create()
  //     {
  //       return Ptr(new Map<WeakToken, V>());
  //     }

} // namespace Scroom::Bookkeeping
