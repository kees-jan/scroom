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
        TokenAddition(Scroom::Bookkeeping::Token t)
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
      
      class Token
      {
      public:
        typedef boost::shared_ptr<Token> Ptr;

      public:
        void add(Stuff s)
        { l.push_back(s); }
        
        void add(const StuffList l)
        { this->l.insert(this->l.end(), l.begin(), l.end()); }

        void merge(StuffList& l)
        { this->l.splice(this->l.end(), l); }

        void merge(Ptr& rhs)
        { merge(rhs->l); }

      public:
        static Scroom::Bookkeeping::Token create()
        { return Scroom::Bookkeeping::Token(Ptr(new Token)); }

      protected:
        Token() {}

      private:
        StuffList l;
      };

      template<typename K, typename V>
      class MapToken : public Token
      {
      public:
        typedef boost::shared_ptr<MapToken<K,V> > Ptr;
        
      private:
        boost::weak_ptr<Scroom::Bookkeeping::MapBase<K,V> > map;
        K k;

      protected:
        MapToken(boost::shared_ptr<Scroom::Bookkeeping::MapBase<K,V> > map, const K& k)
          : map(map), k(k)
        {}
        
      public:
        ~MapToken()
        {
          boost::shared_ptr<Scroom::Bookkeeping::MapBase<K,V> > m = map.lock();
          if(m)
            m->remove(k);
        }

      public:
        static Scroom::Bookkeeping::Token create(boost::shared_ptr<Scroom::Bookkeeping::MapBase<K,V> > map, const K& k)
        { return Scroom::Bookkeeping::Token(Token::Ptr(Ptr(new MapToken<K,V>(map, k)))); }
      };
    }

    ////////////////////////////////////////////////////////////////////////

    inline Token::Token(boost::shared_ptr<Detail::Token> t)
      : boost::shared_ptr<Detail::Token>(t)
    {}

    inline Token::Token(Detail::Token* t)
      : boost::shared_ptr<Detail::Token>(t)
    {}

    inline Token::Token(boost::weak_ptr<Detail::Token> t)
      : boost::shared_ptr<Detail::Token>(t)
    {}

    inline Token::Token()
      : boost::shared_ptr<Detail::Token>(Detail::Token::create())
    {}

    inline Token::Token(Stuff s)
    { get()->add(s); }
    
    inline Token::Token(const StuffList& l)
    { get()->add(l); }
    
    inline void Token::add(Stuff s)
    { get()->add(s); }
    
    inline void Token::add(const StuffList& l)
    { get()->add(l); }

    inline void Token::merge(Token& rhs)
    { get()->merge(rhs); }

    inline void Token::merge(StuffList& l)
    { get()->merge(l); }
      

    inline Detail::TokenAddition& Token::operator+(const Stuff& rhs)
    { return Detail::TokenAddition(*this) + rhs; }

    inline Token& Token::operator+=(const Stuff& rhs)
    { add(rhs); return *this; }
    
    ////////////////////////////////////////////////////////////////////////

    template<typename K, typename V>
    inline const V& MapBase<K,V>::get(const K& k) const
    {
      typename std::map<K,V>::iterator i = map.find(k);

      if(map.end()!=i)
        return i->second;
      else
        throw std::invalid_argument("Invalid key");
    }

    template<typename K, typename V>
    inline V& MapBase<K,V>::get(const K& k)
    {
      typename std::map<K,V>::iterator i = map.find(k);

      if(map.end()!=i)
        return i->second;
      else
        throw std::invalid_argument("Invalid key");
    }

    template<typename K, typename V>
    inline Token MapBase<K,V>::add(const K& k, const V& v)
    {
      if(map.end()!=map.find(k))
        throw std::invalid_argument("Key already exists");

      map[k]=v;

      return Detail::MapToken<K,V>::create(shared_from_this<MapBase<K,V> >(),k);
    }
    
    template<typename K, typename V>
    inline void MapBase<K,V>::remove(const K& k)
    {
      typename std::map<K,V>::iterator i = map.find(k);

      if(map.end()!=i)
        map.erase(i);
    }

    template<typename K, typename V>
    inline void MapBase<K,V>::set(const K& k, const V& v)
    {
      get(k)=v;
    }

    template<typename K, typename V>
    inline std::list<K> MapBase<K,V>::keys() const
    {
      typedef typename std::map<K,V>::value_type value_type;
      
      std::list<K> result;
      BOOST_FOREACH(value_type el, map)
      {
        result.push_back(el.first);
      }
      return result;
    }
    
    template<typename K, typename V>
    inline std::list<V> MapBase<K,V>::values() const
    {
      typedef typename std::map<K,V>::value_type value_type;
      
      std::list<V> result;
      BOOST_FOREACH(value_type el, map)
      {
        result.push_back(el.second);
      }
      return result;
    }
    
    ////////////////////////////////////////////////////////////////////////

    template<typename V>
    inline void Map<Token, V>::addMe(const Token& k, const V& v)
    {
      k->add(add(k,v));
    }
    
    template<typename V>
    inline void Map<WeakToken, V>::addMe(const WeakToken& k, const V& v)
    {
      Token K = k.lock();
      if(K)
        K->add(add(k,v));
      else
        throw std::invalid_argument("boost::weak_ptr can't be locked");
    }
    
    template<typename V>
    inline Token Map<Token, V>::add(const V& v)
    {
      Token k;
      k->add(add(k,v));
      return k;
    }
    
    template<typename V>
    inline Token Map<WeakToken, V>::add(const V& v)
    {
      Token k;
      k->add(add(k,v));
      return k;
    }

    template<typename V>
    inline Token Map<WeakToken, V>::add(const WeakToken& k, const V& v)
    {
      return MapBase<WeakToken, V>::add(k,v);
    }

    template<typename V>
    inline Token Map<Token, V>::add(const Token& k, const V& v)
    {
      return MapBase<Token, V>::add(k,v);
    }

    template<typename K, typename V>
    inline typename Map<K, V>::Ptr Map<K, V>::create()
    {
      return Ptr(new Map<K, V>());
    }

    template<typename V>
    inline typename Map<Token, V>::Ptr Map<Token, V>::create()
    {
      return Ptr(new Map<Token, V>());
    }

    template<typename V>
    inline typename Map<WeakToken, V>::Ptr Map<WeakToken, V>::create()
    {
      return Ptr(new Map<WeakToken, V>());
    }
    
  }
}


#endif
