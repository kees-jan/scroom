/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/observable.hh>

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

class TestObserver
{
public:
  typedef boost::shared_ptr<TestObserver> Ptr;
  typedef boost::weak_ptr<TestObserver> WeakPtr;

  static Ptr create();
};

TestObserver::Ptr TestObserver::create()
{
  return TestObserver::Ptr(new TestObserver());
}

//////////////////////////////////////////////////////////////

class TestObservable: public Observable<TestObserver>
{
public:
  typedef boost::shared_ptr<TestObservable> Ptr;

  std::list<Observer> getObservers();

  static Ptr create();
};

std::list<TestObservable::Observer> TestObservable::getObservers()
{
  return Observable<TestObserver>::getObservers();
}

TestObservable::Ptr TestObservable::create()
{
  return TestObservable::Ptr(new TestObservable());
}

//////////////////////////////////////////////////////////////

class TestRecursiveObservable: public Observable<TestObserver>
{
private:
  TestObservable::Ptr child;

  TestRecursiveObservable(TestObservable::Ptr child);

public:
  typedef boost::shared_ptr<TestRecursiveObservable> Ptr;

  std::list<Observer> getObservers();

  static Ptr create(TestObservable::Ptr child);

protected:
  virtual void observerAdded(Observer const& observer, Scroom::Bookkeeping::Token const& token);
};

TestRecursiveObservable::TestRecursiveObservable(TestObservable::Ptr child_)
  :child(child_)
{
}

std::list<TestRecursiveObservable::Observer> TestRecursiveObservable::getObservers()
{
  return Observable<TestObserver>::getObservers();
}

TestRecursiveObservable::Ptr TestRecursiveObservable::create(TestObservable::Ptr child)
{
  return TestRecursiveObservable::Ptr(new TestRecursiveObservable(child));
}

void TestRecursiveObservable::observerAdded(Observer const& observer, Scroom::Bookkeeping::Token const& token)
{
  token.add(child->registerObserver(observer));
}

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Observable_Tests)

BOOST_AUTO_TEST_CASE(register_observer)
{
  TestObservable::Ptr observable = TestObservable::create();
  TestObserver::Ptr observer = TestObserver::create();
  TestObserver::WeakPtr weakObserver = observer;
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff registration = observable->registerStrongObserver(observer);
  BOOST_CHECK(registration);
  observers = observable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  BOOST_CHECK_EQUAL(observer, observers.front());

  // Observable has a reference to observer, so it doesn't go away
  observer.reset();
  BOOST_CHECK(!observer);
  observers = observable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  observer = weakObserver.lock();
  BOOST_CHECK_EQUAL(observer, observers.front());

  // Unregistering succeeds
  registration.reset();
  observers = observable->getObservers();
  BOOST_REQUIRE(0 == observers.size());
}

BOOST_AUTO_TEST_CASE(register_weak_observer)
{
  TestObservable::Ptr observable = TestObservable::create();
  TestObserver::Ptr observer = TestObserver::create();
  TestObserver::WeakPtr weakObserver = observer;
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff registration = observable->registerObserver(weakObserver);
  BOOST_CHECK(registration);
  observers = observable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  BOOST_CHECK_EQUAL(observer, observers.front());

  // Unregistering succeeds
  registration.reset();
  BOOST_CHECK(!weakObserver.expired());
  observers = observable->getObservers();
  BOOST_REQUIRE(0 == observers.size());
  observer.reset();
  BOOST_CHECK(weakObserver.expired());
}

BOOST_AUTO_TEST_CASE(registered_weak_observer_goes_away)
{
  TestObservable::Ptr observable = TestObservable::create();
  TestObserver::Ptr observer = TestObserver::create();
  TestObserver::WeakPtr weakObserver = observer;
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff registration = observable->registerObserver(observer);
  BOOST_CHECK(registration);
  observers = observable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  BOOST_CHECK_EQUAL(observer, observers.front());

  // Observable does not have a reference to observer, so it goes away
  observers.clear(); // don't forget this reference :-)
  observer.reset();
  BOOST_CHECK(!observer);
  observers = observable->getObservers();
  BOOST_REQUIRE(0 == observers.size());
  BOOST_CHECK(weakObserver.expired());
  BOOST_CHECK(registration);

  // Unregistering succeeds
  registration.reset();
  observers = observable->getObservers();
  BOOST_REQUIRE(0 == observers.size());
}

BOOST_AUTO_TEST_CASE(register_multiple_observers)
{
  TestObservable::Ptr observable = TestObservable::create();
  TestObserver::Ptr o1 = TestObserver::create();
  TestObserver::Ptr o2 = TestObserver::create();
  TestObserver::Ptr o3 = TestObserver::create();
  TestObserver::WeakPtr weakObserver = o1;
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff r1 = observable->registerObserver(weakObserver);
  Stuff r2 = observable->registerObserver(o2);
  Stuff r3 = observable->registerStrongObserver(o3);
  BOOST_CHECK(r1);
  BOOST_CHECK(r2);
  BOOST_CHECK(r3);
  observers = observable->getObservers();
  BOOST_CHECK_EQUAL(3, observers.size());
  observers.remove(o1);
  BOOST_CHECK_EQUAL(2, observers.size());
  observers.remove(o2);
  BOOST_CHECK_EQUAL(1, observers.size());
  observers.remove(o3);
  BOOST_CHECK_EQUAL(0, observers.size());

  // Unregistering succeeds
  r1.reset();
  observers = observable->getObservers();
  BOOST_CHECK_EQUAL(2, observers.size());
  observers.remove(o1);
  BOOST_CHECK_EQUAL(2, observers.size());
  observers.remove(o2);
  BOOST_CHECK_EQUAL(1, observers.size());
  observers.remove(o3);
  BOOST_CHECK_EQUAL(0, observers.size());

  r2.reset();
  observers = observable->getObservers();
  BOOST_CHECK_EQUAL(1, observers.size());
  observers.remove(o1);
  BOOST_CHECK_EQUAL(1, observers.size());
  observers.remove(o2);
  BOOST_CHECK_EQUAL(1, observers.size());
  observers.remove(o3);
  BOOST_CHECK_EQUAL(0, observers.size());

  r3.reset();
  observers = observable->getObservers();
  BOOST_CHECK_EQUAL(0, observers.size());
}

BOOST_AUTO_TEST_CASE(register_observer_multiple_times)
{
  TestObservable::Ptr observable = TestObservable::create();
  TestObserver::Ptr observer = TestObserver::create();
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff r1 = observable->registerStrongObserver(observer);
  Stuff r2 = observable->registerStrongObserver(observer);
  BOOST_CHECK(r1);
  BOOST_CHECK(r2);
  BOOST_CHECK_EQUAL(r1, r2);
  observers = observable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  BOOST_CHECK_EQUAL(observer, observers.front());

  // Unregistering succeeds
  r1.reset();
  observers = observable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  BOOST_CHECK_EQUAL(observer, observers.front());
  r2.reset();
  observers = observable->getObservers();
  BOOST_REQUIRE(0 == observers.size());
}

BOOST_AUTO_TEST_CASE(register_weak_observer_multiple_times)
{
  TestObservable::Ptr observable = TestObservable::create();
  TestObserver::Ptr observer = TestObserver::create();
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff r1 = observable->registerObserver(observer);
  Stuff r2 = observable->registerObserver(observer);
  BOOST_CHECK(r1);
  BOOST_CHECK(r2);
  BOOST_CHECK_EQUAL(r1, r2);
  observers = observable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  BOOST_CHECK_EQUAL(observer, observers.front());

  // Unregistering succeeds
  r1.reset();
  observers = observable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  BOOST_CHECK_EQUAL(observer, observers.front());
  r2.reset();
  observers = observable->getObservers();
  BOOST_REQUIRE(0 == observers.size());
}

BOOST_AUTO_TEST_CASE(register_observer_recursively)
{
  TestObservable::Ptr observable = TestObservable::create();
  TestRecursiveObservable::Ptr recursiveObservable = TestRecursiveObservable::create(observable);
  TestObserver::Ptr observer = TestObserver::create();
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff registration = recursiveObservable->registerObserver(observer);
  BOOST_CHECK(registration);
  observers = observable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  BOOST_CHECK_EQUAL(observer, observers.front());
  observers = recursiveObservable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  BOOST_CHECK_EQUAL(observer, observers.front());

  // Unregistering succeeds
  registration.reset();
  observers = observable->getObservers();
  BOOST_REQUIRE(0 == observers.size());
  observers = recursiveObservable->getObservers();
  BOOST_REQUIRE(0 == observers.size());
}

BOOST_AUTO_TEST_CASE(shared_from_this)
{
  TestObservable::Ptr original = TestObservable::create();
  TestObservable::Ptr copy1 = original->shared_from_this<TestObservable>();
  boost::shared_ptr<TestObservable const> copy2 = original;
  boost::shared_ptr<TestObservable const> copy3 = copy2->shared_from_this<TestObservable>();

  BOOST_CHECK_EQUAL(original, copy1);
  BOOST_CHECK_EQUAL(copy2, copy3);
  BOOST_CHECK_EQUAL(original, copy2);
}

BOOST_AUTO_TEST_CASE(deleting_observable_deletes_observer)
{
  TestObservable::Ptr observable = TestObservable::create();
  TestObserver::Ptr observer = TestObserver::create();
  TestObserver::WeakPtr weakObserver = observer;
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff registration = observable->registerStrongObserver(observer);
  BOOST_CHECK(registration);

  // Observable has a reference to observer, so it doesn't go away
  observer.reset();
  BOOST_CHECK(!observer);
  observers = observable->getObservers();
  BOOST_REQUIRE(1 == observers.size());
  observer = weakObserver.lock();
  BOOST_CHECK_EQUAL(observer, observers.front());
  observers.clear();

  // Destroying observable destroys observer
  observer.reset();
  BOOST_CHECK(!observer);
  observable.reset();
  BOOST_CHECK(!observable);
  BOOST_CHECK(!weakObserver.lock());
}

BOOST_AUTO_TEST_SUITE_END()
