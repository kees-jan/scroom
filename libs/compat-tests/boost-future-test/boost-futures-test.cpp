// Adapted from boost-futures-example.cpp

#include <boost/future.hpp>
#include <boost/future_stream.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include <boost/test/unit_test.hpp>


using boost::future;
using boost::promise;
using boost::shared_ptr;
using boost::future_wrapper;

class JobQueue1 {
  public:
    template <class T>
      future<T> schedule(boost::function<T (void)> const& fn) {
        boost::mutex::scoped_lock lck(mutex_);
        promise<T> prom; // create promise
        q_.push_back(future_wrapper<T>(fn, prom)); //queue the job
        condition_.notify_all(); // wake worker thread(s)
        return future<T>(prom); // return a future created from the promise
      }
    void exec_loop();
    JobQueue1() : waiting_for_god_(false), active_threads_(0) {}
    ~JobQueue1() { // we must kill thread before we're dead
      boost::mutex::scoped_lock lck(mutex_);
      waiting_for_god_ = true; // flag to tell thread to die
      condition_.notify_all();
      while (active_threads_) // all threads must exit
        condition_.wait(lck);
    }
  protected:
    bool waiting_for_god_;
    int active_threads_;
    std::list<boost::function<void ()> > q_;
    boost::mutex mutex_;
    boost::condition condition_; // signal we wait for when queue is empty
};

void JobQueue1::exec_loop() {
  boost::mutex::scoped_lock lck(mutex_);
  ++active_threads_;
  while (true) {
    while ((q_.size() == 0) && !waiting_for_god_)
      condition_.wait(lck); // wait for a job to be added to queue
    if (waiting_for_god_) {
      --active_threads_;
      condition_.notify_all();
      return;
    }
    boost::function<void ()> f = q_.front();
    q_.pop_front();
    lck.unlock(); // unlock the queue while we exec the job
    f(); // call the future_wrapper functor
    lck.lock();
  }
}

/// Motivating Example ///
int add(int a, int b) {
  return a+b;
}

void f1() {
  JobQueue1 q;
  boost::thread t(boost::bind(&JobQueue1::exec_loop, &q));
  future<int> fut = q.schedule<int>(boost::bind(add, 11, 13));
  BOOST_CHECK_EQUAL(24, fut.get());
}

/// Promises, Promises Example ///
void f2() {
  promise<int> prom; //create a promise
  future<int> fut(prom); //create the associated future
  BOOST_CHECK(!fut.ready()); // future is not ready yet
  prom.set(42); // set the future value
  BOOST_CHECK(fut.ready()); // the future is SET
  BOOST_CHECK_EQUAL(42, fut.get());
}

/// Reference Semantics ///
void f3() {
  promise<int> p1;
  promise<int> p2(p1); // p2 refers to same object as p1
  future<int> f1(p1); // refers to same future object as p1 and p2
  future<int> f2(p2); // again, all these refer to the same objects
  p2.set(99);
  BOOST_CHECK_EQUAL(f1.get(), f2.get()); // true
  BOOST_CHECK_EQUAL(f1.get(), 99); //see, I told you 
}

/// Exception Handling and Transport ///
void f4() {
  promise<long> p1;
  future<long> f1;
  p1.set_exception(std::runtime_error("Darn Error"));
  BOOST_CHECK_THROW(f1.get(), boost::broken_promise);
  BOOST_CHECK(f1.has_exception());
}

char GetChar(std::string const& s, unsigned int pos) {
  return s.at(pos); // will throw std::out_of_range if pos > s.size()
}

void f5() {
  JobQueue1 q;
  boost::thread t(boost::bind(&JobQueue1::exec_loop, &q));

  std::string s("Hi!");
  future<char> f = q.schedule<char>(boost::bind(GetChar, s, 999)); //oops!
  BOOST_CHECK_THROW(f.get(), std::out_of_range);
  BOOST_CHECK(f.has_exception());
}

/// Broken Promises ///
future<int> make_future() {
  promise<int> prom;
  return future<int>(prom);
} //oops, future created but promise goes out of scope here!

void f6() {
  future<int> f = make_future();
  BOOST_CHECK_THROW(f.get(), boost::broken_promise);
}

///Future Type Conversion ///
void f7() {
  promise<long> plong;
  future<long> flong(plong);
  future<int> fint = flong;
  plong.set(27L);
  BOOST_CHECK_EQUAL(27, fint.get());
}

/// future<void> ///
void f8() {
  promise<int> pi;
  future<int> fi(pi);
  future<void> fv = fi;

  BOOST_CHECK(!fi.ready());
  BOOST_CHECK(!fv.ready());
  pi.set(42);
  BOOST_CHECK(fi.ready());
  BOOST_CHECK_EQUAL(42, fi.get());
  BOOST_CHECK(fv.ready());
}

/// Callbacks ///
void callfunc(future<int> f, bool* done) {
  BOOST_CHECK_EQUAL(33, f.get());
  *done = true;
}

void f9() {
  promise<int> pi;
  future<int> f(pi);
  bool done = false;
  boost::callback_reference cb_ref1 = f.add_callback(boost::bind(callfunc, f, &done));
  BOOST_CHECK(!done);
  pi.set(33); 
  BOOST_CHECK(done);
}

/// Guards ///
class JobQueue3 : public JobQueue1 {
  public:
    template <class T>
      void queueWrapped(boost::function<void (void)> fn) {
        boost::mutex::scoped_lock lck(mutex_);
        q_.push_back(fn);
        condition_.notify_all();
      }

    template <class T>
      future<T> schedule(boost::function<T (void)> const& fn, future<void> guard = future<void>()) {
        promise<T> prom; // create promise
        future_wrapper<T> wrap(fn,prom); 
        guard.add_callback(boost::bind(&JobQueue3::queueWrapped<T>, this, wrap));
        return future<T>(prom); // return a future created from the promise
      }
};

int Add(int a, int b) {
  return a+b;
}

void f10() {
  JobQueue3 q;
  boost::thread t1(boost::bind(&JobQueue3::exec_loop, &q));
  boost::thread t2(boost::bind(&JobQueue3::exec_loop, &q));

  promise<int> p9;
  future<int> f9(p9);
  future<int> f8 = q.schedule<int>(boost::bind(Add, f9, 3), f9); 
  p9.set(99);
  BOOST_CHECK_EQUAL(f8.get(), 102);

  future<int> fa = q.schedule<int>(boost::bind(Add, 1, 2));
  future<int> fb = q.schedule<int>(boost::bind(Add, fa, 3), fa); // fa + 3, Guarded on completion of a
  future<int> fc = q.schedule<int>(boost::bind(Add, fb, 4), fb); // fb + 4, Guarded on completion of b
  BOOST_CHECK_EQUAL(10, fc.get());
}

/// Lazy Futures ///
class JobQueue4 : public JobQueue3 {
  public:
    template<class T>
      future<T> lazy_schedule(boost::function<T (void)> const& fn) {
        promise<T> prom; // create promise
        future<void> guard = prom.get_needed_future(); // we treat this as the guard
        future_wrapper<T> wrap(fn,prom); 
        guard.add_callback(boost::bind(&JobQueue3::queueWrapped<T>, this, wrap));
        return future<T>(prom); // return a future created from the promise
      }
};

int Mult(int a, int b) {
  return a*b;
}

void f11() {
  JobQueue4 q;
  boost::thread t1(boost::bind(&JobQueue4::exec_loop, &q));
  future<float> squares[100];
  for (int i=0; i<100; ++i)
    squares[i] = q.lazy_schedule<float>(boost::bind(Mult, i, i)); // this won't execute until we need the result

  // print just a few select squares, which will be computed as we go
  for (int i=0; i<100; i+=7)
  {
    BOOST_CHECK_EQUAL(i*i, squares[i].get());
  }
  // anything we didn't ask for was not computed, like squares[5]
  BOOST_CHECK(!squares[5].ready());
}

/// Future Streams ///

// Hand Rolled Streams
template<class T>
struct item {
  typedef shared_ptr<item<T> > item_p;
  item(const T & val, const future<item_p> nxt) : value(val), next(nxt) {}
  T value;
  future<item_p> next;
};

void producer1(promise<shared_ptr<item<int> > > head) {
  for (int i=0; i<24; ++i) {
    promise<shared_ptr<item<int> > > p;
    head.set(shared_ptr<item<int> >(new item<int>(i, p)));
    head = p;
  }
}

void f12() {
  promise<shared_ptr<item<int> > > head;
  future<shared_ptr<item<int> > > next(head);
  producer1(head);
  head.reset(); // break the promise of any more
  // Consume the results and add them together
  int accum=0;
  try { // stream is closed when next promise is broken.
    while (true) {
      accum += next.get()->value; // will block
      next = next.get()->next; // move on to next item
    }
  } catch (boost::broken_promise &) {}
  BOOST_CHECK_EQUAL(276, accum);
}

// future_stream

// produces 23 int values
void producer(boost::promise_stream<int> pstream) {
  for (int i=0; i<24; ++i)
    pstream.send(i);
  pstream.close();
}

void f13() {
  boost::promise_stream<int> pstream;
  boost::future_stream<int> fstream(pstream); // construct future_stream from promise_stream
  // get pointer to head of stream BEFORE producer starts
  boost::future_stream<int>::iterator iter = fstream.begin();
  // Launch producer in another thread
  boost::thread t1(boost::bind(producer, pstream));
  // Consume the results and add them together
  int accum=0;
  for (; (iter != fstream.end()); ++iter)
    accum += *iter;
  BOOST_CHECK_EQUAL(276, accum);
  t1.join();
}

boost::unit_test_framework::test_suite* init_unit_test_suite(int, char*[])
{
  boost::unit_test_framework::test_suite* test =
    BOOST_TEST_SUITE("Scroom: futures test suite");

  test->add(BOOST_TEST_CASE(f1));
  test->add(BOOST_TEST_CASE(f2));
  test->add(BOOST_TEST_CASE(f3));
  test->add(BOOST_TEST_CASE(f4));
  test->add(BOOST_TEST_CASE(f5));
  test->add(BOOST_TEST_CASE(f6));
  test->add(BOOST_TEST_CASE(f7));
  test->add(BOOST_TEST_CASE(f8));
  test->add(BOOST_TEST_CASE(f9));
  test->add(BOOST_TEST_CASE(f10));
  test->add(BOOST_TEST_CASE(f11));
  test->add(BOOST_TEST_CASE(f12));
  test->add(BOOST_TEST_CASE(f13));

  return test;
}
