Principles
==========

Solid principles
----------------

The solid principles are a set of five principles that'll guide you towards a good design.
You should probably google them.
There's much documentation on-line, written by people who can explain it better than me.
I'll give it a (brief) shot anyway 😉

*Single responsibility principle*
  The idea that everything, every class, every method, every variable, should have exactly one responsibility.
  This will guide you into splitting things up into small, reusable parts.
  The same idea is the core of the Unix philosophy: "Do one thing, and do it well".

*Open/Closed principle*
  The idea that adding new features should be done by adding code, not by modifying existing code.
  If you manage to do this, you'll know that you have not broken any existing functionality by adding the new feature, simply because you didn't touch any of the existing code.
  In practice, however, this is hard to do, since it requires you to foresee all future extensions (and see `YAGNI`_).
  Still, when adding a new feature, it is a good practice to first do a refactoring step, where you change the structure of the existing code,
  such that the feature (and others like it) can be added by adding new code only.

*Liskov substitution principle*
  Named after `Barbara Liskov <https://en.wikipedia.org/wiki/Barbara_Liskov>`_.
  The idea that you should only inherit from something (or implement an interface) if you can meet all the expectations that users of that base class will have of it.
  That is to say, your implementation should not do too much, or too little.
  The classical example of doing too little is to throw a "not supported" exception, or show a messagebox to that effect.

*Interface segregation principle*
  The idea that interfaces should be as small as possible.
  If a client can't reasonably implement a method, then that method probably shouldn't be in the interface.
  If a user doesn't need a particular method, then that method probably shouldn't be in the interface.

*Dependency inversion principle*
  Generic code should not depend on specific code, on implementation details.
  Instead, generic code should depend on generic interfaces that are implemented by more specific code.
  This way, the dependency is inverted.
  The specific code now depends on the generic interface.
  This, for example, applies when injecting dependencies to improve testability.
  (Hardware) abstraction layers are another example of this principle in action.

YAGNI
-----

Acronym for "You ain't gonna need it":
Do not overcomplicate your code by introducing generalisations or extensions points that you don't know you'll need.


