# 0087: The module object cache grows without bound

- Type: tooling
- Status: open
- Area: compiler (C and selfhost), `axiom cache`

`<cache>/obj/` (§16.7) is never evicted: `axiom cache clean` is the only way to shrink it.
Every compiler that builds a module adds an object under a new key. That includes each change to
the compiler itself, since the compiler is part of the cache key, and each build flag or CPU.
Working on the self-hosted compiler, about ten agents rebuilt the benchmark and
the test suites with their own compilers for a day. The cache reached 13 GB in 4,846 objects
and filled the disk, so builds and tests failed with "no space left on device". Objects
older than an hour accounted for 12.3 GB of that, and no compiler in use would ever ask for them again.

Proposal (the spec lists cache eviction as an open follow-up of §16.7):

- Record each object's last use: touch it, or set its mtime, when a build takes it from the cache.
- After a build, if the cache is over a size limit, delete the least recently used objects
  until it is under the limit. The limit defaults to a few GB and is set with
  `$AXIOM_CACHE_LIMIT` or `axiom cache limit <size>`.
- Deletion must be safe while other builds run: write each object to a temporary name and rename it into place, as now.
  A concurrent build that loses an object it has just checked recompiles it.
- `axiom cache` reports the size and the number of objects.
