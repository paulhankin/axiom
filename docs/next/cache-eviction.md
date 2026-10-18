## Compiler

- The module object cache (spec §16.7) is kept under a size limit: after a build,
  the least recently used objects over `$AXIOM_CACHE_LIMIT` (2 GiB by default; `0`
  for no limit) are removed, except those used in the last hour. `axiom cache size`
  reports the cache's size and object count.
