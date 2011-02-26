ramalloc, release 0.
====================

**ramalloc** is a parallelized, amortized-constant time allocator for objects smaller than a hardware page. it exhibits deterministic performance characteristics, which is necessary for soft real-time applications. it also reduces memory fragmentation, which is important for long-running processes. *ramalloc* is designed for game and interpreter development but should be useful for other applications as well. *ramalloc* is also parallelized; there is no global resource that requires serialized access.

if you choose to integrate *ramalloc* into your application, please read this entire document before starting. an improper integration is likely to leak an unacceptable amount of memory.

features
--------

* deterministic performance (amortized-constant time).
* low fragmentation.
* limited waste (a 4 byte object can have less than 1 byte of overhead).
* coexists with a supplimentary heap allocator.
* query any valid base pointer to determine source allocator, if any.
* returns unused memory to the system.
* parallelized.

history
-------

*ramalloc* is based on an allocator i wrote for the MMORPG [Horizons][1], before it was released in 2003. the intention its design was to reduce the allocation rate that the many interpreters imposed on the (global) heap allocator, while eliminating obstacles to parallelism.

requirements
------------

* a C89 compiler (to-date, only MSVC has been tested).
* [CMake][2].
* support for threads.
* Windows (a Linux port is forthcoming).

license
-------

*ramalloc* source code is provided under the *New BSD License*.

this means that you are free to use *ramalloc* for any purpose you see fit, as long as you give proper credit for my work.

you're not required to distribute source code if you use *ramalloc*. if you don't, you can satisfy the credit requirement by crediting me in the about box (in a GUI application), a usage message (in a command-line application), or in a credits list (in a game).

see the file LICENSE in the source distribution for more details.

usage
-----

*ramalloc* diverges from traditional memory allocation semantics in order to better serve the needs of parallelism.

traditional allocators provide a two-operation interface: *allocate* and *deallocate*. *ramalloc*, in order to minimize competition for resources basically provides a three-operation interface: **acquire**, **discard**, and **reclaim**.

* the *acquire* operation allocates memory and usually *reclaims* a small amount of discarded memory.

* the *discard* operation informs the allocator that memory can be *reclaimed*. this is the only operation that used with memory allocated by a different thread.

* the *reclaim* operation deallocates discarded memory.

these operations can easily be mapped onto a traditional *allocate/deallocate* interface, and are. the likelihood of asymmetrical allocation patterns, however, requires that each allocating thread service *ramalloc* with *reclaim* operations regularly (e.g. as part of the event loop).

*ramalloc* provides additional operations that are also useful:

* a *query* operation tells whether a given address was acquired from *ramalloc* and the size, if indeed it was. queries do not crash the application if the provided address was acquired from some sort of allocator, making this operation useful for layering *ramalloc* on top of a supplimentary allocator.

* a *flush* operation reclaims memory more quickly than *reclaim* but at the expense of deterministic performance.

important considerations
------------------------

*ramalloc*, unlike other allocators, will leak memory if you do not periodically service it with *reclaim* operations from each memory-producing thread. this might seem inconvenient but makes it possible to perform most operations without serializing threads.

other considerations exist that you should be aware of when integrating any custom allocator such as *ramalloc*.

should an error such as a buffer overrun occur, *ramalloc* would more likely corrupt memory than crash the process. this is because *ramalloc* does not use guard bytes or guard pages. therefore, i recommend that you reserve *ramalloc* for release builds or refrain from integrating *ramalloc* until you have thouroughly debugged your application. otherwise, you risk hiding bugs not yet discovered.

also, i do not recommend using *ramalloc* with a tool such as *IBM Rational Purify*, *valgrind*, or *HeapAgent*. these are fantastic tools but unless integrated though a tool-specific API, memory pooling strategies hide memory errors from these tools. i recommend that you prepare a specific build for use with these tools that uses only the default system allocator.

status
------

i consider *ramalloc* to be of alpha quality. 

*ramalloc* is complete, however, and should be bug free. i am strict about error checking (and reporting) and there are numerous tests, each validating a different module in the library. 

still, *ramalloc* has not yet been tested in a real application. once it has, and once i've worked through enough outstanding issues, i'll announce a beta release.

known issues
------------

* *ramalloc* is poorly documented. this is my first priority, in the absence of unanticipated bugs.

* currently, *ramalloc* only builds on Windows platforms. i have made an effort to make porting as simple as possible and i intend to provide a Linux port when i've finished documenting.

* *ramalloc* is not yet optimized. the allocator should be attractive nonetheless because it should not pause processing for an undesirable period of time to no matter how high your allocation rate is. 

* *ramalloc* requires threads that allocate to be running until they have no  outstanding allocations remaining. this makes *ramalloc* incompatible with applications that create and destroy threads with any sort of frequency.  please consider using a thread pool until i am able to address this issue.

* *ramalloc* leaks memory stored in thread local storage. while sloppy, it is a small, finite quantity of memory that should be harmless if leaked. 


-----

**README.md for ramalloc** by [michael lowell roberts][3].
copyright &copy; 2011, michael lowell roberts.
all rights reserved. 
licensed under the *New BSD License*. 

[1]: http://en.wikipedia.org/wiki/Istaria:_Chronicles_of_the_Gifted
[2]: http://www.cmake.org
[3]: http://fmrl.org
