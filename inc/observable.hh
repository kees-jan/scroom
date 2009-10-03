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
  boost::unique_lock<boost::mutex> lock(mut);
  return observers;
}

template<typename T>
void Observable<T>::registerObserver(T* observer)
{
  boost::unique_lock<boost::mutex> lock(mut);
  observers.push_back(observer);
}

template<typename T>
void Observable<T>::unregisterObserver(T* observer)
{
  boost::unique_lock<boost::mutex> lock(mut);
  observers.remove(observer);
}

#endif
