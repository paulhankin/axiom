## Standard library

- `std/mem`: with threads, each thread keeps a cache of free small objects per size
  class, refilled from its heap and flushed back in batches (objects freed by another
  thread are cached too and go back to their heap together), so that most allocations
  and frees take no lock. A thread takes the least used of up to 32 heaps (four per
  processor) instead of one chosen by its stack address, which could put several
  threads on one heap. On a 4-core machine, four threads allocating and freeing their
  own small objects ran about 30 times as fast as before, and producer/consumer pairs
  (objects freed by another thread) about 1.3 to 2 times.
- `std/mem` gives free memory back to the system once it has been unused for about ten
  seconds, gradually (native targets), not only past a 64 MiB budget; empty spans are
  reused from free blocks instead of growing the heap; heaps refill from their lowest
  spans first. A workload that built up 200 MiB of small objects and kept a tenth went
  from 175 MiB resident to 69 MiB after ten seconds; building and freeing linked
  structures of 10 MiB repeatedly ran in 11 MiB instead of 71 MiB, twice as fast.
- `mem.stats()` returns the allocator's counters: bytes of small and large objects in
  use, cached in threads, taken from the system, free but resident, given back, and the
  caches' refills and flushes.
- Code that starts threads without `std/thread` must now call `mem.thread_exit()` on each
  such thread before it ends (as well as `mem.enable_threads()` before the first).
