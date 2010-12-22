#ifndef BOOST_FUTURE_HPP
#define BOOST_FUTURE_HPP 1

//  Copyright (c) 2007 Braddock Gaskill Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//  exception_ptr.hpp/cpp copyright Peter Dimov

// Adapted for usage in Scroom by Kees-Jan Dijkzeul

#include <boost/shared_ptr.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/utility/result_of.hpp>

#include <boost/exception_ptr.hpp>
#include <boost/future_exceptions.hpp>
#include <boost/future_detail.hpp>

#ifdef __GNUC__
#define DEPRECATED __attribute__ ((deprecated))
#else
#define DEPRECATED
#endif

namespace boost {

  class callback_reference;

  class untyped_promise {
    public:
      untyped_promise() {}
      explicit untyped_promise(const shared_ptr<detail::future_impl> &fimpl) : f_(fimpl) {}
      untyped_promise(const untyped_promise &other) : f_(other.f_) {}
      untyped_promise& operator=(const untyped_promise& t) {f_ = t.f_; return *this;}
      template<typename E> void set_exception( E const & e ) { // stores the exception e and transitions to ready()
        // f_->set_exception(detail::copy_exception(e));
        f_->set_exception(copy_exception<E>(e));
      }

      // Attempt's a 'throw', assuming there is an exception set
      void set_current_exception() {
        f_->set_exception(current_exception());
      }

      void set_exception( const exception_ptr & e) {
        f_->set_exception(e);
      }

      // Add a cancel handler, which will be invoked if
      // future&lt;T&gt;::cancel() is ever called
      // Note: If the future is ALREADY canceled, then get_cancel_handler()
      // will call the handler immediately.
      // There is only one cancel_handler() for an underlying Future instance
      void DEPRECATED set_cancel_handler( const boost::function<void (void)> &f ) {
        f_->set_cancel_handler(f);
      }

      void DEPRECATED unset_cancel_handler() {
        f_->set_cancel_handler(boost::function<void (void)>());
      }

    protected:
      shared_ptr<detail::future_impl> f_;
  };

  template<typename R> class promise : public untyped_promise
  {
    public:
      promise() : impl_(new detail::promise_impl<R>) {f_ = impl_->f_;}; // creates an empty future
      promise(const promise& t) : untyped_promise(t.impl_->f_), impl_(t.impl_) {}
      promise& operator=(const promise& t) {
        impl_ = t.impl_; 
        untyped_promise::operator=(t);
        return *this;
      }

      void set( R const & r ) { // sets the value r and transitions to ready()
        impl_->f_->set_value(r, *impl_->value_);
      }

      void set_or_throw( R const & r) {
        if (!impl_->f_->set_value(r, *impl_->value_))
          throw future_already_set();
      }

      bool is_needed() {return impl_->f_->is_needed();}
      void wait_until_needed() {return impl_->f_->wait_until_needed();}
      shared_ptr<detail::future_impl> get_needed_future() {return impl_->f_->get_needed_future();}

      void reset() {
        impl_.reset();
        f_.reset();
      }
    private:
      template <typename Y> friend class future;
      shared_ptr<detail::promise_impl<R> > impl_;
  };

  template<typename R> class future
  {
    public:
      //future() : impl_(new detail::future_impl<R>) {}; // creates an empty future

      // Default constructor will create a future, and will immediately set a
      // broken_promise exception.
      // A default-constructed future is only good for equality assignment to a
      // valid future.
      future() : impl_(), value_() {
        promise<R> p;
        impl_ = p.impl_->f_;
        value_ = p.impl_->value_;
      }

      future(const future& t) : impl_(t.impl_), value_(t.value_) {}

      template<typename T>
        future(const future<T>& t) : 
          impl_(t.impl_), 
          value_(new detail::return_value_type_adaptor<R,T>(t.value_)) 
    {}

      future(const promise<R>& p) : impl_(p.impl_->f_), value_(p.impl_->value_) {}

      template<typename T>
        future(const promise<T>& p) 
        : impl_(p.impl_->f_), 
        value_(new detail::return_value_type_adaptor<R,T>(p.impl_->value_)) 
    {}

    protected:
      // used by future<void> for typeless futures
      future(const boost::shared_ptr<detail::future_impl> &impl) :
        impl_(impl),
        value_(new detail::return_value_real<R>(-999)) //value is never used
    {}
    public:

      future& operator=(const future& t) {
        impl_ = t.impl_; 
        value_ = t.value_;
        return *this;
      }

      template<typename T>
        future<R>& operator=(const future<T>& t) {
          impl_ = t.impl_; 
          value_ = shared_ptr<detail::return_value_base<R> >(new detail::return_value_type_adaptor<R,T>(t.value_));
          return *this;
        }
      ~future() {};

      bool has_value() const { // newer Dimov proposal N2185
        return impl_->has_value();
      }

      bool has_exception() const { // N2185
        return impl_->has_exception();
      }

      bool ready() const { // queries whether the future contains a value or an exception
        return impl_->ready();
      }

      void wait() { // wait for ready()
        return impl_->wait();
      }

      bool timed_wait( boost::xtime const & abstime ) {
        return impl_->timed_wait(abstime);
      }

      operator R() const { // N2185
        return impl_->get(*value_);
      }

      R get() const {
        return impl_->get(*value_);
      }

      R operator()() const { // waits for a value, then returns it
        return impl_->get(*value_);
      }

      void set_needed() const {
        impl_->set_needed();
      }

      // set future exception to boost::future_cancel, and call
      // the cancel handler if one has been set by the user
      void DEPRECATED cancel() {
        impl_->cancel();
      }

      callback_reference add_callback(const boost::function<void (void)> &f) {
        return impl_->add_callback(f);
      }

      // remove_callback will remove a registered callback
      // Calling with an invalid callback_reference, or a
      // callback_reference which has already been removed is
      // undefined.
      // This function is guaranteed not to return until the
      // callback is removed.
      // This can block if callbacks are already in progress
      void remove_callback(callback_reference &ref) {
        impl_->remove_callback(ref);
      }
    private:
      template <typename Y> friend class future;
      shared_ptr<detail::future_impl > impl_;
      shared_ptr<detail::return_value_base<R> > value_;
  };

  // note, promise<int> must be public for friend to work in specialization (?)
  template<> class promise<void> : public promise<int> {
    private:
      typedef promise<int> base_type;
    public:
      promise() : promise<int>() {}
      promise(const promise& t) : promise<int>(t) {}
      promise& operator=(const promise& t) {
        base_type::operator=(t);
        return *this;
      }
      using base_type::set_exception;
      using base_type::set_cancel_handler;
      using base_type::is_needed;
      using base_type::wait_until_needed;
      void set() {
        base_type::set(0);
      }
      void set_or_throw() {
        base_type::set_or_throw(0);
      }
  };

  // void specialization, based on Peter Dimov's example
  template<> class future<void> : private future<int> {
    private:
      typedef future<int> base_type;
    public:
      future() : base_type() {}
      future(const future& t) : base_type((const future<int>&)t) {}
      future(const promise<void> &p) : base_type((const promise<int>&)p) {}
      future(const boost::shared_ptr<detail::future_impl> &impl) : base_type(impl) {}

      template<typename T>
        future(const future<T> &t) : base_type(t.impl_) {}

      template<typename T>
        future(const promise<T> &t) : base_type(t.impl_->f_) {} 

      future& operator=(const future& t) {
        base_type::operator=((const future<int>&)t);
        return *this;
      }

      template<typename T>
        future& operator=(const future<T>& t) {
          future<void> tmp(t);
          base_type::operator=((const future<int>&)tmp);
          return *this;
        }

      using base_type::has_value;
      using base_type::has_exception;
      using base_type::timed_wait;
      using base_type::cancel;
      using base_type::ready;
      using base_type::wait;
      using base_type::set_needed;
      using base_type::add_callback;
      using base_type::remove_callback;

      void get() const {
        base_type::get();
      }
  };

  template <typename R > class promise< R& > : public promise<R*> {
    private:
      typedef promise< R* > base_type;
    public:
      promise() : promise<R*>() {}
      promise(const promise& t) : promise<R*>(t) {}
      promise& operator=(const promise& t) {
        base_type::operator=(t);
        return *this;
      }
      using base_type::set_exception;
      using base_type::set_cancel_handler;
      using base_type::is_needed;
      using base_type::wait_until_needed;
      void set(R &r) {
        base_type::set(&r);
      }
      void set_or_throw(R &r) {
        base_type::set_or_throw(&r);
      }
  };

  // reference passing specialization, based on Peter Dimov's example
  template<typename R > class future< R& >: private future< R* >
  {
    private:
      typedef future< R* > base_type;
    public:
      future() : base_type() {}
      future(const future& t) : base_type((const future<R*> &) t) {}
      future(const promise<R*> &p) : base_type((const promise<R*> &) p) {}
      future& operator=(const future& t) {
        base_type::operator=(t);
        return *this;
      }
      using base_type::has_value;
      using base_type::has_exception;
      using base_type::timed_wait;
      using base_type::cancel;
      using base_type::ready;
      using base_type::wait;
      using base_type::add_callback;
      using base_type::set_needed;

      operator R&() const {
        return *base_type::get();
      }

      R& get() const {
        return *base_type::get();
      }
  };

  template<typename R> class future_wrapper
  {
    public:
      future_wrapper(const boost::function<R (void)> &fn, const promise<R> &ft ) : fn_(fn), ft_(ft) {}; // stores fn and ft
      void operator()() throw() { // executes fn() and places the outcome into ft
        try {
          ft_.set(fn_());
        } catch (...) {
          ft_.set_exception(current_exception());
        }
      }
      future<R> get_future() const {return future<R>(ft_);}
    private:
      boost::function<R (void)> fn_;
      promise<R> ft_;
  };

  // void specialization
  template<> class future_wrapper<void>
  {
    public:
      future_wrapper(const boost::function<void (void)> &fn, const promise<void> &ft ) : ft_(ft), fn_(fn) {}; // stores fn and ft
      void operator()() throw() { // executes fn() and places the outcome into ft
        try {
          fn_();
          ft_.set();
        } catch (...) {
          ft_.set_exception(current_exception());
        }
      }
      future<void> get_future() const {return future<void>(ft_);}
    private:
      promise<void> ft_;
      boost::function<void (void)> fn_;
  };
} // end of namespace boost

#endif // BOOST_FUTURE_HPP 

