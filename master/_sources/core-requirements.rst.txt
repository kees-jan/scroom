Core requirements
=================

Be fast
-------

Being fast breaks down into two parts:

* Low latency: Never block the user, show at least something as soon as possible
* High throughput: Process large amounts of data as fast as possible

There is a balance to be struck here. 
Optimizing too much leads to code that is poorly readable and poorly maintainable.
As a rule, we focus on choosing smart algorithms, as opposed to optimizing the code itself.

Low latency
^^^^^^^^^^^

Where possible, if the user requests a long-running operation, we run it in the background.
That also means we should be able to cancel those background jobs if the user changes his mind.

Low latency is the reason that when we first load a bitmap, the windows shows the top-left corner at 1:1 zoom.
That is the data that is (usually) available first, so we allow the user to look at/work with that while we load the rest.

Low latency is also the reason we do a lot of preprocessing when we load a bitmap. 
That allows us to quickly show the data the user requests later.

High throughput
^^^^^^^^^^^^^^^

High throughput is achieved by using all available cores wherever possible/required.
This is done by splitting any task into smaller chunks, and submitting them on the :scroom:`CpuBound` threadpool by calling its :scroom:`ThreadPool::schedule` method.
Be sure to supply a :scroom:`queue <Queue>` object when you call :scroom:`ThreadPool::schedule`.
If you later delete the :scroom:`queue <Queue>` object, the work you scheduled will be canceled.

80/20 rule
----------

I'm sure you've heard of the 80/20 rule: The idea that the last 20% of the work (requirements) take 80% of the time.
Here, we try to invert that rule by just doing the 20% of work that generates 80% user happyness.

This rule is actually the reason for the `be fast`_ rule, above.
I hate waiting.
Not having to do it makes me very happy with relatively little effort 😉.
It is also the reason for Scroom not looking very polished.
Scroom tends to serve a technical audience that prefers features and convenience over a slick look, and looking slick does tend to take time.

Derived requirements
====================

No popups
---------

Let's start with the exception to the rule:
If you need to show *one* window *immediately* and *in response to something the user did*, that's probably ok.
Examples of this are obviously the File->Open dialog, but also the metadata window supported by some bitmap formats.
Nearly everything else, like error popups, confirmation boxes, questions to the user are forbidden.

Don't confuse the user
  The user may have many Scroom windows open at the same time.
  If one of them opens a popup, you're leaving the user with the puzzle to which Scroom window the popup belongs.

`Be fast`_
  Popups tend to block be blocking. 
  That means that all processing stops, including processing for other Scroom windows that have nothing to do with the popup being shown.

So, to reiterate: 
You cannot show popups or errors while loading a bitmap. 
The user may have requested 20 (or more) bitmaps to be loaded.
If he did, loading will be done sequentially, in the background.
If you were to show a (blocking) popup, the user would not know to which Scroom windows the popup belongs, and you would delay the loading of the other bitmaps.





