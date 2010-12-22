#ifndef FUTURE_EXCEPTIONS_HPP_ 
#define FUTURE_EXCEPTIONS_HPP_

#include <stdexcept>

namespace boost {
  struct broken_promise : public std::exception {
    virtual const char *what() const throw () {
      return "Broken Promise Exception";
    }
  };

  struct future_already_set : public std::exception {
    virtual const char *what() const throw () {
      return "Future Already Set Exception";
    }
  };

  struct future_cancel : public std::exception {
    virtual const char *what() const throw () {
      return "Future Canceled Exception";
    }
  };
} // namespace

#endif //FUTURE_EXCEPTIONS_HPP_
