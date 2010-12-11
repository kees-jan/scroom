/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

#ifndef OBSERVABLE_HH
#define OBSERVABLE_HH

#include<list>

#include <boost/thread.hpp>

template<typename T>
class Observable : boost::noncopyable
{
private:
  std::list<T*> observers;
  boost::mutex mut;
  
protected:
  std::list<T*> getObservers();
  
public:
  Observable();
  
  void registerObserver(T* observer);
  void unregisterObserver(T* observer);
};

template<typename T>
Observable<T>::Observable()
{
}

template<typename T>
std::list<T*> Observable<T>::getObservers()
{
  boost::mutex::scoped_lock lock(mut);
  return observers;
}

template<typename T>
void Observable<T>::registerObserver(T* observer)
{
  boost::mutex::scoped_lock lock(mut);
  observers.push_back(observer);
}

template<typename T>
void Observable<T>::unregisterObserver(T* observer)
{
  boost::mutex::scoped_lock lock(mut);
  observers.remove(observer);
}

#endif
