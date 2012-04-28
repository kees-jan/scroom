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

#ifndef BOOKKEEPINGIMPL_HH
#define BOOKKEEPINGIMPL_HH

#include <stdexcept>

#include <boost/foreach.hpp>

namespace Scroom
{
  namespace Bookkeeping
  {
    namespace Detail
    {
      class TokenAddition : public Scroom::Bookkeeping::Token
      {
      public:
        TokenAddition(const Scroom::Bookkeeping::Token& t)
          : Scroom::Bookkeeping::Token(t)
        {}

        TokenAddition& operator+(const Stuff& rhs)
        { add(rhs); return *this; }

        TokenAddition& operator+=(const Stuff& rhs)
        { add(rhs); return *this; }

        TokenAddition& operator+(TokenAddition& rhs)
        { merge(rhs); return *this; }

        TokenAddition& operator+=(TokenAddition& rhs)
        { merge(rhs); return *this; }
      };
      
      class TokenImpl
      {
      public:
        typedef boost::shared_ptr<TokenImpl> Ptr;

      public:
        void add(const Stuff& s)
        { l.push_back(s); }
        
        void add(const StuffList l)
        { this->l.insert(this->l.end(), l.begin(), l.end()); }

        void merge(StuffList& l)
        { this->l.splice(this->l.end(), l); }

        void merge(Ptr& rhs)
        { merge(rhs->l); }

      public:
        static Scroom::Bookkeeping::Token create()
        { return Scroom::Bookkeeping::Token(Ptr(new TokenImpl)); }

      protected:
        TokenImpl() {}

      private:
        StuffList l;
      };

      template<typename K, typename V>
      class MapTokenImpl : public TokenImpl
      {
      public:
        typedef boost::shared_ptr<MapTokenImpl<K,V> > Ptr;
        
      private:
        boost::weak_ptr<Scroom::Bookkeeping::MapBase<K,V> > map;
        WeakToken t;
        K k;

      protected:
        MapTokenImpl(boost::shared_ptr<Scroom::Bookkeeping::MapBase<K,V> > map, const K& k)
          : map(map), k(k)
        {}
        
      public:
        ~MapTokenImpl()
        {
          boost::shared_ptr<Scroom::Bookkeeping::MapBase<K,V> > m = map.lock();
          if(m)
            m->remove(k, t);
        }

      public:
        static Scroom::Bookkeeping::Token create(boost::shared_ptr<Scroom::Bookkeeping::MapBase<K,V> > map, const K& k)
        {
          Ptr t = Ptr(new MapTokenImpl<K,V>(map, k));
          t->t = t;
          return Scroom::Bookkeeping::Token(TokenImpl::Ptr(t));
        }
      };

      template<typename V>
      class ValueType
      {
      public:
        typedef boost::shared_ptr<ValueType<V> > Ptr;
        typedef boost::weak_ptr<ValueType<V> > WeakPtr;

      public:
        V value;
        WeakToken token;

      protected:
        ValueType(V value)
          : value(value)
        {}
        
      public:
        static Ptr create(V value)
        { return Ptr(new ValueType<V>(value)); }
      };

      template<typename V>
      class LValue
      {
      public:
        typedef typename ValueType<V>::Ptr VTPtr;
        
      private:
        VTPtr pv;

      public:
        LValue(VTPtr pv)
          :pv(pv)
        {}

        LValue& operator=(const V& v)
        { pv->value = v; return *this; }

        operator V()
        { return pv->value; }
      };
    }

    ////////////////////////////////////////////////////////////////////////

    inline Token::Token(const boost::shared_ptr<Detail::TokenImpl>& t)
      : boost::shared_ptr<Detail::TokenImpl>(t)
    {}

    inline Token::Token(const boost::weak_ptr<Detail::TokenImpl>& t)
      : boost::shared_ptr<Detail::TokenImpl>(t)
    {}

    inline Token::Token()
      : boost::shared_ptr<Detail::TokenImpl>(Detail::TokenImpl::create())
    {}

    inline Token::Token(const Stuff& s)
      : boost::shared_ptr<Detail::TokenImpl>(Detail::TokenImpl::create())
    { get()->add(s); }
    
    inline Token::Token(const StuffList& l)
      : boost::shared_ptr<Detail::TokenImpl>(Detail::TokenImpl::create())
    { get()->add(l); }
    
    inline void Token::add(const Stuff& s)
    { get()->add(s); }
    
    inline void Token::add(const StuffList& l)
    { get()->add(l); }

    inline void Token::merge(Token& rhs)
    { get()->merge(rhs); }

    inline void Token::merge(StuffList& l)
    { get()->merge(l); }
      

    inline Detail::TokenAddition Token::operator+(const Stuff& rhs)
    { return Detail::TokenAddition(*this) + rhs; }

    inline Token& Token::operator+=(const Stuff& rhs)
    { add(rhs); return *this; }
    
    ////////////////////////////////////////////////////////////////////////

    template<typename K, typename V>
    inline Token MapBase<K,V>::reserve(const K& k)
    {
      boost::mutex::scoped_lock lock(mut);
      if(map.end()!=map.find(k))
        throw std::invalid_argument("Key already exists");

      typename Detail::ValueType<V>::Ptr pv = Detail::ValueType<V>::create(V());
      map[k]=pv;

      Token t = Detail::MapTokenImpl<K,V>::create(shared_from_this<MapBase<K,V> >(),k);
      t.add(pv);
      pv->token = t;
      return t;
    }
    
    template<typename K, typename V>
    inline Token MapBase<K,V>::reReserve(const K& k)
    {
      boost::mutex::scoped_lock lock(mut);
      typename MapType::iterator i = map.find(k);

      if(map.end()==i)
      {
        map[k]=typename Detail::ValueType<V>::WeakPtr();
        i = map.find(k);
      }
      
      typename Detail::ValueType<V>::Ptr pv = i->second.lock();
      if(!pv)
      {
        pv = Detail::ValueType<V>::create(V());
        i->second = pv;
      }

      Token t = pv->token.lock();
      if(!t)
      {
        t = Detail::MapTokenImpl<K,V>::create(shared_from_this<MapBase<K,V> >(),k);
        t.add(pv);
        pv->token = t;
      }

      return t;
    }

    template<typename K, typename V>
    inline void MapBase<K,V>::remove(const K& k, WeakToken wt)
    {
      boost::mutex::scoped_lock lock(mut);
      typename MapType::iterator i = map.find(k);

      if(map.end()!=i)
      {
        typename Detail::ValueType<V>::Ptr pv = i->second.lock();
        if(pv)
        {
          Token t = wt.lock();
          Token t_orig = pv->token.lock();
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

    template<typename K, typename V>
    inline void MapBase<K,V>::remove(const K& k)
    {
      boost::mutex::scoped_lock lock(mut);
      typename MapType::iterator i = map.find(k);

      if(map.end()!=i)
        map.erase(i);
    }

    template<typename K, typename V>
    inline Detail::LValue<V> MapBase<K,V>::at(const K& k)
    {
      boost::mutex::scoped_lock lock(mut);
      typename MapType::iterator i = map.find(k);

      if(map.end()!=i)
      {
        typename Detail::ValueType<V>::Ptr pv = i->second.lock();
        if(pv)
          return Detail::LValue<V>(pv);
      }

      throw std::invalid_argument("Invalid key");
    }

    template<typename K, typename V>
    inline void MapBase<K,V>::set(const K& k, const V& v)
    {
      boost::mutex::scoped_lock lock(mut);
      typename MapType::iterator i = map.find(k);

      if(map.end()!=i)
      {
        typename Detail::ValueType<V>::Ptr pv = i->second.lock();
        if(pv)
        {
          pv->value=v;
          return;
        }
      }

      throw std::invalid_argument("Invalid key");
    }
    
    template<typename K, typename V>
    inline V MapBase<K,V>::get(const K& k)
    {
      boost::mutex::scoped_lock lock(mut);
      typename MapType::iterator i = map.find(k);

      if(map.end()!=i)
      {
        typename Detail::ValueType<V>::Ptr pv = i->second.lock();
        if(pv)
        {
          return pv->value;
        }
      }

      throw std::invalid_argument("Invalid key");
    }
    
    template<typename K, typename V>
    inline std::list<K> MapBase<K,V>::keys() const
    {
      boost::mutex::scoped_lock lock(mut);
      std::list<K> result;
      BOOST_FOREACH(typename MapType::value_type el, map)
      {
        result.push_back(el.first);
      }
      return result;
    }
    
    template<typename K, typename V>
    inline std::list<V> MapBase<K,V>::values() const
    {
      boost::mutex::scoped_lock lock(mut);
      std::list<V> result;
      BOOST_FOREACH(typename MapType::value_type el, map)
      {
        typename Detail::ValueType<V>::Ptr pv = el.second.lock();
        if(pv)
          result.push_back(pv->value);
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

    template<typename K, typename V>
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
   
  }
}


#endif