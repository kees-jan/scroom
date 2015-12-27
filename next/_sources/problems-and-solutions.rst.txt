Problems and solutions
======================

Static initialization order
---------------------------

A `static` the word we use in C++ to indicate global variables. It derives 
from "statically allocated" memory, which means that it is allocated at 
compile time, and not on the stack or the heap. A `static` variable is 
initialized before `main()` gets called, and destroyed after it finishes.

The static initialization order fiasco is a common problem in C++ where 
the order of initialization of static variables across different translation
units is undefined. This means that if you use one global to initialize another,
the first global might not be initialized yet, leading to undefined behavior.

The solution to this is to first of all not use global variables. But if you must,
wrap them in a function. This is called the "construct on first use" idiom. 
The language guarantees that the static variable will be initialized thread safely
the first time the function is called, and that it will be destroyed when the program
exits. This way, you can ensure that the variable is initialized before it is used.

Take this example from the :scroom:`ThreadList::instance()` method:

.. code-block:: cpp

  ThreadList::Ptr ThreadList::instance()
  {
    static Ptr const threadList = std::make_shared<ThreadList>();
    return threadList;
  }

Static destruction
------------------

The other side of the coin is that static variables will eventually be destroyed
when the program exits. Since scroom is multithreaded and asynchronous, any code 
runs the risk of executing after ``main()`` has finished. Therefore, no code
should rely on global variables still being set, except during initialization/construction.

All classes should be self-sufficient
-------------------------------------

Because scroom is :ref:`asynchronous <asynchronous>`, your parent class might already be destroyed
when your asynchronous code runs. Either because scroom has been shut down and the global variables 
have been destroyed, or because the current document has been closed, or both.

Therefore, any scheduled tasks should contain everything they need to do their work, without 
relying on other objects or globals to still exist.

This is why memory is often managed using shared pointers. This way, many other objects can
keep a thing alive if they depend on it. It is a common pattern that an object in its constructor
initializes shared pointers to its dependencies, and then uses those exclusively, instead of
assuming a global variable still exists.

So there's two reasons for writing self-sufficient classes:

1. To handle `Static destruction`_
2. To handle :ref:`asynchronous execution <asynchronous>`