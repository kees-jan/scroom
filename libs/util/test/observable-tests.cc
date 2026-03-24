/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <memory>
#include <utility>

#include <gtest/gtest.h>

#include <scroom/observable.hh>

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

class TestObserver
{
public:
  using Ptr = std::shared_ptr<TestObserver>;
  using WeakPtr = std::weak_ptr<TestObserver>;

  static Ptr create();
};

TestObserver::Ptr TestObserver::create() { return std::make_shared<TestObserver>(); }

//////////////////////////////////////////////////////////////

class TestObservable : public Observable<TestObserver>
{
public:
  using Ptr = std::shared_ptr<TestObservable>;

  std::list<Observer> getObservers();

  static Ptr create();
};

std::list<TestObservable::Observer> TestObservable::getObservers() { return Observable<TestObserver>::getObservers(); }

TestObservable::Ptr TestObservable::create() { return std::make_shared<TestObservable>(); }

//////////////////////////////////////////////////////////////

class TestRecursiveObservable : public Observable<TestObserver>
{
private:
  TestObservable::Ptr child;

  explicit TestRecursiveObservable(TestObservable::Ptr child);

public:
  using Ptr = std::shared_ptr<TestRecursiveObservable>;

  std::list<Observer> getObservers();

  static Ptr create(TestObservable::Ptr child);

protected:
  void observerAdded(Observer const& observer, Scroom::Bookkeeping::Token const& token) override;
};

TestRecursiveObservable::TestRecursiveObservable(TestObservable::Ptr child_)
  : child(std::move(child_))
{
}

std::list<TestRecursiveObservable::Observer> TestRecursiveObservable::getObservers()
{
  return Observable<TestObserver>::getObservers();
}

TestRecursiveObservable::Ptr TestRecursiveObservable::create(TestObservable::Ptr child)
{
  return TestRecursiveObservable::Ptr(new TestRecursiveObservable(std::move(child)));
}

void TestRecursiveObservable::observerAdded(Observer const& observer, Scroom::Bookkeeping::Token const& token)
{
  token.add(child->registerObserver(observer));
}

//////////////////////////////////////////////////////////////

TEST(Observable_Tests, register_observer) // NOLINT
{
  TestObservable::Ptr const observable = TestObservable::create();
  TestObserver::Ptr observer = TestObserver::create();
  TestObserver::WeakPtr const weakObserver = observer;
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff registration = observable->registerStrongObserver(observer);
  EXPECT_TRUE(registration);
  observers = observable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  EXPECT_EQ(observer, observers.front());

  // Observable has a reference to observer, so it doesn't go away
  observer.reset();
  EXPECT_FALSE(observer);
  observers = observable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  observer = weakObserver.lock();
  EXPECT_EQ(observer, observers.front());

  // Unregistering succeeds
  registration.reset();
  observers = observable->getObservers();
  ASSERT_TRUE(observers.empty());
}

TEST(Observable_Tests, register_weak_observer) // NOLINT
{
  TestObservable::Ptr const observable = TestObservable::create();
  TestObserver::Ptr observer = TestObserver::create();
  TestObserver::WeakPtr const weakObserver = observer;
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff registration = observable->registerObserver(weakObserver);
  EXPECT_TRUE(registration);
  observers = observable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  EXPECT_EQ(observer, observers.front());

  // Unregistering succeeds
  registration.reset();
  EXPECT_FALSE(weakObserver.expired());
  observers = observable->getObservers();
  ASSERT_TRUE(observers.empty());
  observer.reset();
  EXPECT_TRUE(weakObserver.expired());
}

TEST(Observable_Tests, registered_weak_observer_goes_away) // NOLINT
{
  TestObservable::Ptr const observable = TestObservable::create();
  TestObserver::Ptr observer = TestObserver::create();
  TestObserver::WeakPtr const weakObserver = observer;
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff registration = observable->registerObserver(observer);
  EXPECT_TRUE(registration);
  observers = observable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  EXPECT_EQ(observer, observers.front());

  // Observable does not have a reference to observer, so it goes away
  observers.clear(); // don't forget this reference :-)
  observer.reset();
  EXPECT_FALSE(observer);
  observers = observable->getObservers();
  ASSERT_TRUE(observers.empty());
  EXPECT_TRUE(weakObserver.expired());
  EXPECT_TRUE(registration);

  // Unregistering succeeds
  registration.reset();
  observers = observable->getObservers();
  ASSERT_TRUE(observers.empty());
}

TEST(Observable_Tests, register_multiple_observers) // NOLINT
{
  TestObservable::Ptr const observable = TestObservable::create();
  TestObserver::Ptr const o1 = TestObserver::create();
  TestObserver::Ptr const o2 = TestObserver::create();
  TestObserver::Ptr const o3 = TestObserver::create();
  TestObserver::WeakPtr const weakObserver = o1;
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff r1 = observable->registerObserver(weakObserver);
  Stuff r2 = observable->registerObserver(o2);
  Stuff r3 = observable->registerStrongObserver(o3);
  EXPECT_TRUE(r1);
  EXPECT_TRUE(r2);
  EXPECT_TRUE(r3);
  observers = observable->getObservers();
  EXPECT_EQ(3, observers.size());
  observers.remove(o1);
  EXPECT_EQ(2, observers.size());
  observers.remove(o2);
  EXPECT_EQ(1, observers.size());
  observers.remove(o3);
  EXPECT_EQ(0, observers.size());

  // Unregistering succeeds
  r1.reset();
  observers = observable->getObservers();
  EXPECT_EQ(2, observers.size());
  observers.remove(o1);
  EXPECT_EQ(2, observers.size());
  observers.remove(o2);
  EXPECT_EQ(1, observers.size());
  observers.remove(o3);
  EXPECT_EQ(0, observers.size());

  r2.reset();
  observers = observable->getObservers();
  EXPECT_EQ(1, observers.size());
  observers.remove(o1);
  EXPECT_EQ(1, observers.size());
  observers.remove(o2);
  EXPECT_EQ(1, observers.size());
  observers.remove(o3);
  EXPECT_EQ(0, observers.size());

  r3.reset();
  observers = observable->getObservers();
  EXPECT_EQ(0, observers.size());
}

TEST(Observable_Tests, register_observer_multiple_times) // NOLINT
{
  TestObservable::Ptr const observable = TestObservable::create();
  TestObserver::Ptr const observer = TestObserver::create();
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff r1 = observable->registerStrongObserver(observer);
  Stuff r2 = observable->registerStrongObserver(observer);
  EXPECT_TRUE(r1);
  EXPECT_TRUE(r2);
  EXPECT_EQ(r1, r2);
  observers = observable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  EXPECT_EQ(observer, observers.front());

  // Unregistering succeeds
  r1.reset();
  observers = observable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  EXPECT_EQ(observer, observers.front());
  r2.reset();
  observers = observable->getObservers();
  ASSERT_TRUE(observers.empty());
}

TEST(Observable_Tests, register_weak_observer_multiple_times) // NOLINT
{
  TestObservable::Ptr const observable = TestObservable::create();
  TestObserver::Ptr const observer = TestObserver::create();
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff r1 = observable->registerObserver(observer);
  Stuff r2 = observable->registerObserver(observer);
  EXPECT_TRUE(r1);
  EXPECT_TRUE(r2);
  EXPECT_EQ(r1, r2);
  observers = observable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  EXPECT_EQ(observer, observers.front());

  // Unregistering succeeds
  r1.reset();
  observers = observable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  EXPECT_EQ(observer, observers.front());
  r2.reset();
  observers = observable->getObservers();
  ASSERT_TRUE(observers.empty());
}

TEST(Observable_Tests, register_observer_recursively) // NOLINT
{
  TestObservable::Ptr const observable = TestObservable::create();
  TestRecursiveObservable::Ptr const recursiveObservable = TestRecursiveObservable::create(observable);
  TestObserver::Ptr const observer = TestObserver::create();
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff registration = recursiveObservable->registerObserver(observer);
  EXPECT_TRUE(registration);
  observers = observable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  EXPECT_EQ(observer, observers.front());
  observers = recursiveObservable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  EXPECT_EQ(observer, observers.front());

  // Unregistering succeeds
  registration.reset();
  observers = observable->getObservers();
  ASSERT_TRUE(observers.empty());
  observers = recursiveObservable->getObservers();
  ASSERT_TRUE(observers.empty());
}

TEST(Observable_Tests, shared_from_this) // NOLINT
{
  TestObservable::Ptr const original = TestObservable::create();
  TestObservable::Ptr const copy1 = original->shared_from_this<TestObservable>();
  std::shared_ptr<TestObservable const> const copy2 = original;
  std::shared_ptr<TestObservable const> const copy3 = copy2->shared_from_this<TestObservable>();

  EXPECT_EQ(original, copy1);
  EXPECT_EQ(copy2, copy3);
  EXPECT_EQ(original, copy2);
}

TEST(Observable_Tests, deleting_observable_deletes_observer) // NOLINT
{
  TestObservable::Ptr observable = TestObservable::create();
  TestObserver::Ptr observer = TestObserver::create();
  TestObserver::WeakPtr const weakObserver = observer;
  std::list<TestObservable::Observer> observers;

  // Registration succeeds
  Stuff const registration = observable->registerStrongObserver(observer);
  EXPECT_TRUE(registration);

  // Observable has a reference to observer, so it doesn't go away
  observer.reset();
  EXPECT_FALSE(observer);
  observers = observable->getObservers();
  ASSERT_TRUE(1 == observers.size());
  observer = weakObserver.lock();
  EXPECT_EQ(observer, observers.front());
  observers.clear();

  // Destroying observable destroys observer
  observer.reset();
  EXPECT_FALSE(observer);
  observable.reset();
  EXPECT_FALSE(observable);
  EXPECT_FALSE(weakObserver.lock());
}
