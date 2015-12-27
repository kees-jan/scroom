Patterns
========

Formatting
----------

C++ Formatting is done by ``clang-format-10``. 
Formatting for ``CMakeLists.txt`` is done using ``cmake-format`` (``pip install cmake-format``). 
Formatting is checked during the CI build.

Be sure to format your code before committing. 
Commits that only change formatting are not likely to be accepted. 
Most IDEs show the commit message next to the code, and a message that reads "Updated formatting" does not really help the develpers after you.

Hence, I recommend you configure your IDE to format on every save. 
Or maybe use a ``pre-commit`` hook. 
Scroom doesn't currently have one,though, so patches are very welcome 😉


Clang-tidy
----------

We use ``clang-tidy-10`` to enforce a consistent style and find common bugs. 
This, too, is checked during the CI build.

``clang-tidy`` currently reports a fair amount of warnings. 
The idea is to reduce this number over time.

Note that ``clang-tidy`` is a relative new development within Scroom. 
If a warning turns out to not make sense, turning it off project-wide is certainly an option.


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



