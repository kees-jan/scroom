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

#ifndef BOOKKEEPING_HH
#define BOOKKEEPING_HH

#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/utilities.hh>

namespace Scroom
{
  namespace Bookkeeping
  {
    namespace Detail
    {
      class Token;
    }
    
    typedef boost::shared_ptr<Detail::Token> Token;

    template<typename K, typename V>
    class MapBase : public virtual Scroom::Utils::Base
    {
    private:
      std::map<K,V> map;

    public:
      Token add(const K& k, const V& v);
      void remove(const K& k);
      void set(const K& k, const V& v);
      const V& get(const K& k) const;
      V& get(const K& k);

      // void addMe(const K& k, const V& v);
      // Token add(const V& v);
    };

    template<typename K, typename V>
    class Map : public MapBase<K,V> {};

    template<typename V>
    class Map<Token, V> : public MapBase<Token,V>
    {
    public:
      void addMe(const Token& k, const V& v);
      Token add(const V& v);
    };
    
    template<typename V>
    class Map<boost::weak_ptr<Detail::Token>, V> : public MapBase<boost::weak_ptr<Detail::Token>,V>
    {
    public:
      void addMe(const boost::weak_ptr<Detail::Token>& k, const V& v);
      Token add(const V& v);
    };
    
  }
}


#include <scroom/impl/bookkeepingimpl.hh>


#endif
