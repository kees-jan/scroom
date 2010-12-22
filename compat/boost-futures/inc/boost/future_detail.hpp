#ifndef BOOST_FUTURE_DETAIL_HPP
#define BOOST_FUTURE_DETAIL_HPP 1

#include <boost/shared_ptr.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/function.hpp>
#include <vector>
#include <list>

// DEBUG
#include <iostream>

namespace boost {
  namespace detail {
    class future_impl;
  }

  class callback_reference {
    public:
      callback_reference() : empty_(true) {}
      callback_reference(const callback_reference &t) : empty_(t.empty_), iter_(t.iter_) {}
      bool operator==(callback_reference const& t) {
	if (t.empty_ != empty_) return false;
	if (t.empty_ && empty_) return true;
	return (t.iter_ == iter_);
      }
    private:
      friend class detail::future_impl;
      bool empty_;
      std::list<boost::function<void (void)> >::iterator iter_;
  };

  template<typename R> class future;
  template<typename R> class promise;
  namespace detail {
    template<typename T>
      class return_value_base {
	public:
	  virtual T get() const = 0;
	  virtual ~return_value_base() {}
      };

    template<typename T>
      class return_value_real : public return_value_base<T> {
	public:
          return_value_real() {}
          explicit return_value_real(const T &value)
            : value_(new T(value)) {}
          
	  virtual T get() const {
	    return *value_;
	  }
	  void set(const T& value) {
	    value_ = shared_ptr<T>(new T(value));
	  }
	  virtual ~return_value_real() {}
	private:
	  shared_ptr<T> value_;
      };

    template<typename T,typename U>
      class return_value_type_adaptor : public return_value_base<T> {
	public:
	  return_value_type_adaptor(const shared_ptr<return_value_base<U> > &real_value) : value_(real_value) {}
	  virtual T get() const {
	    return value_->get();
	  }
          virtual ~return_value_type_adaptor() {}
	private:
	  shared_ptr<return_value_base<U> > value_;
      };

    class future_impl {
      public:
	future_impl() : has_value_(false), has_exception_(false), is_canceled_(false), callbacks_in_progress_(false), is_needed_() {}
	bool has_value() const {
	  boost::mutex::scoped_lock lck(mutex_);
	  return has_value_;
	}
	bool has_exception() const {
	  boost::mutex::scoped_lock lck(mutex_);
	  return has_exception_;
	}
	bool ready() const {
	  boost::mutex::scoped_lock lck(mutex_);
	  return (has_value_ || has_exception_);
	}
	void wait() {
	  set_needed();
	  boost::mutex::scoped_lock lck(mutex_);
	  while (!has_value_ && !has_exception_)
	    cond_.wait(lck);
	  return;
	}
	bool timed_wait( boost::xtime const & abstime ) {
	  set_needed();
	  mutex::scoped_lock lck(mutex_);
	  while (!has_value_ && !has_exception_)
	    if (!cond_.timed_wait(lck, abstime))
	      return false; /* timeout */
	  return true;
	}
	// Could return by-ref if set_value only called once
	template <typename R>
	  R get(const return_value_base<R> &value) {
	    set_needed();
	    boost::mutex::scoped_lock lck(mutex_);
	    while (!has_value_ && !has_exception_)
	      cond_.wait(lck);
	    if (has_exception_)
	      rethrow_exception(exception_);
	    return value.get();
	  }

	template <typename R>
	  bool set_value( const R &r, return_value_real<R> &value) {
	    boost::mutex::scoped_lock lck(mutex_);
	    if (has_value_ || has_exception_) return false;
	    value.set(r);
	    has_value_ = true;
	    notify(lck);
	    return true;
	  }

	void set_exception( const exception_ptr &e) {
	  boost::mutex::scoped_lock lck(mutex_);
	  if (has_value_ || has_exception_) return;
	  exception_ = e;
	  has_exception_ = true;
	  notify(lck);
	}
	void cancel() {
	  boost::mutex::scoped_lock lck(mutex_);
	  if (has_value_ || has_exception_) return; // ignore 
	  exception_ = boost::copy_exception(future_cancel());
	  has_exception_ = true;
	  is_canceled_ = true;
	  boost::function<void (void)> canhan = cancel_handler_;
	  notify(lck); //unlocks mutex, also deletes cancel_handler_
	  canhan(); 
	}
	void end_promise() {
	  boost::mutex::scoped_lock lck(mutex_);
	  if (has_value_ || has_exception_) return; // ignore 
	  exception_ = boost::copy_exception(broken_promise());
	  has_exception_ = true;
	  notify(lck);
	}
	boost::callback_reference add_callback(const boost::function<void (void)> f) {
	  boost::mutex::scoped_lock lck(mutex_);
          if (has_value_ || has_exception_) {
	    lck.unlock(); // never call a callback within the mutex
	    f(); // future already fulfilled.  Call the callback immediately.
	    return callback_reference(); //return empty callback_reference
	  }
	  callbacks_.push_front(f);
          boost::callback_reference cb_ref;
          cb_ref.iter_ = callbacks_.begin();
	  cb_ref.empty_ = false;
          return cb_ref;
	}
        void remove_callback(const boost::callback_reference &ref) {
	  boost::mutex::scoped_lock lck(mutex_);
          if (callbacks_in_progress_) {
	    while (callbacks_in_progress_)
	      cond_.wait(lck);
	    //notify already removed all callbacks
            return;
          }
          if (has_value_ || has_exception_) return; //ignore, already set, and automatically removed
	  callbacks_.erase(ref.iter_);
        }
	bool set_cancel_handler( const boost::function<void (void)> &f ) {
	  boost::mutex::scoped_lock lck(mutex_);
	  if (is_canceled_) {
	    lck.unlock();
	    f();
	    return false;
	  }
	  if (has_value_ || has_exception_ || callbacks_in_progress_) 
	    return false; //ignore, future already set, cancel will never happen
	  cancel_handler_ = f;
	  return true;
	}

	shared_ptr<future_impl> get_needed_future() const {
	  boost::mutex::scoped_lock lck(mutex_);
	  if (!is_needed_) // allocate if desired
	    is_needed_.reset(new future_impl);
	  return is_needed_;
	}

	// as-needed functionality permits lazy eval and as-needed producer/consumer
	void set_needed() {
	  shared_ptr<future_impl> n = get_needed_future();
	  n->set();
	}

	bool is_needed() const {
	  boost::mutex::scoped_lock lck(mutex_);
	  // if we are bound, we always say we are already needed
	  return ((is_needed_ && is_needed_->ready()) || has_value_ || has_exception_);
	}
	void wait_until_needed() const {
	  shared_ptr<future_impl> n = get_needed_future();
	  n->wait();
	}
      private:
	typedef std::list<boost::function<void (void)> > func_list_t;
	void notify(boost::mutex::scoped_lock &lck) {
	  callbacks_in_progress_ = true;
	  cond_.notify_all();
	  func_list_t cb(callbacks_);
	  lck.unlock();
	  func_list_t::iterator it;
	  for (it = cb.begin(); it != cb.end(); ++it)
	    (*it)();
          // delete all callbacks - they will never be needed again
          // that is also why this clear is thread-safe outside the mutex
          callbacks_.clear();
	  cancel_handler_ = boost::function<void (void)>();
	  // the below is in case someone tried to remove while we are calling
	  boost::mutex::scoped_lock lck2(mutex_);
	  callbacks_in_progress_ = false;
	  cond_.notify_all();
	}
	bool set() { // a very simple set, used for as_needed_ future
	  boost::mutex::scoped_lock lck(mutex_);
	  if (has_value_ || has_exception_) return false;
	  has_value_ = true;
	  notify(lck);
	  return true;
	}
	bool has_value_;
	bool has_exception_;
	bool is_canceled_;
	exception_ptr exception_;
	mutable boost::mutex mutex_;
	mutable boost::condition cond_;
	func_list_t callbacks_;
	bool callbacks_in_progress_;
	mutable shared_ptr<future_impl> is_needed_;
	boost::function<void (void)> cancel_handler_;
    };

    template<typename R> 
      class promise_impl {
	public:
	  promise_impl() : f_(new future_impl), value_(new return_value_real<R>) {};
	  ~promise_impl() {
	    f_->end_promise();
	  }
	  shared_ptr<detail::future_impl> f_;
	  shared_ptr<return_value_real<R> > value_;
      };

  } // namespace detail
} // end of namespace boost
#endif // BOOST_FUTURE_DETAIL_HPP
