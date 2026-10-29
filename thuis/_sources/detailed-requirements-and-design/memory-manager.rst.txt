Memory manager
==============

Bitmaps in scroom are typically very large. Therefore data that is currently not needed is stored
compressed. This is what a :scroom:`MemoryBlob <Scroom::MemoryBlobs::Blob>` represents. If you call
:scroom:`get() <Scroom::MemoryBlobs::Blob::get()>` or
:scroom:`getConst() <Scroom::MemoryBlobs::Blob::getConst()>`, the memory is decompressed until you
release the shared pointer. If you called :scroom:`get() <Scroom::MemoryBlobs::Blob::get()>`, then after
release the memory is considered "dirty" and will be recompressed on the :scroom:`ThreadPool <CpuBound()>`
at priority ``COMPRESS_PRIO``.

Returning memory to the OS
--------------------------

We want compressed storage to return to the OS when MemoryBlobs are freed. Small allocations
can remain in allocator-managed heaps; large allocations can be released separately, using
anonymous mappings on Linux or virtual memory allocations on Windows.

See the NOTES section of the `malloc(3) manpage <https://man7.org/linux/man-pages/man3/malloc.3.html#NOTES>`_:
glibc distinguishes the heap, grown using ``sbrk()``, from a "private anonymous mapping" created
using ``mmap()`` for allocations above ``MMAP_THRESHOLD``. It also describes "memory allocation
arenas" used to reduce contention between threads.

:scroom:`PageProvider <Scroom::MemoryBlobs::PageProvider>` therefore groups pages into backing
allocations large enough to favor this behavior, rather than allocating each compressed fragment
separately.
