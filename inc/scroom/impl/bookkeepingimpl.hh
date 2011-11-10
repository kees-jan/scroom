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

#include <scroom/stuff.hh>

namespace Scroom
{
  namespace Bookkeeping
  {
    
    namespace Detail
    {
      using Scroom::Utils::Stuff;
      using Scroom::Utils::StuffList;

      class Token
      {
      public:
        
        typedef boost::shared_ptr<Token> Ptr;

        static Ptr create()
        {
          return Ptr(new Token());
        }
        
        static Ptr create(Stuff s)
        {
          Ptr p = create();
          p->add(s);
          return p;
        }
        
        static Ptr create(const StuffList l)
        {
          Ptr p = create();
          p->add(l);
          return p;
        }

      public:
        void add(Stuff s)
        {
          l.push_back(s);
        }
        
        void add(const StuffList l)
        {
          this->l.insert(this->l.end(), l.begin(), l.end());
        }

      protected:
        Token() {}

      private:
        StuffList l;
      };

      template<typename K, typename V>
      class MapToken : public Token
      {
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
        
        static boost::shared_ptr<MapToken<K,V> > create(boost::shared_ptr<Scroom::Bookkeeping::MapBase<K,V> > map, const K& k)
        { return boost::shared_ptr<MapToken<K,V> >(new MapToken<K,V>(map, k)); }
      };
    }

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
      Token k = Detail::Token::create();
      k->add(add(k,v));
      return k;
    }
    
    template<typename V>
    inline Token Map<WeakToken, V>::add(const V& v)
    {
      Token k = Detail::Token::create();
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
