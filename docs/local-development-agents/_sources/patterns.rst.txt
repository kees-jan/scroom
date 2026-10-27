Patterns
========

RAII
----

`RAII (Resource Acquisition Is Initialization) <https://en.cppreference.com/w/cpp/language/raii>`_ is a pattern in C++ where a "resource" is acquired in the constructor of a class, and released in its destructor.
According to the "Single responsibility principle", that class should do nothing else, except manage that resource.
The goal is to ensure that the resource is always freed.
Freeing a resource is easy to forget, for example in case of exceptions, or when there are many paths through your code (though that's a bad thing in itself 😉)

The RAII pattern is commonly used for managing memory (i.e. shared or unique pointers), files (``std::ostream``), or for acquiring mutexes (``std::lock_guard``).
Scroom takes the approach of using RAII wherever possible. 
For example :scroom:`when registering an observer <Scroom::Utils::Observable::registerStrongObserver>`, or when *(example needed)*.

We're not entirely consistent, though. :scroom:`Viewable`, for example, currently isn't RAII-style, but it should be. There are probably more examples of this.

The Threadpool
--------------


create() methods
----------------



