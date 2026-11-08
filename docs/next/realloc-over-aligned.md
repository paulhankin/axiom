## Fixes

- `std/mem`: `realloc` of an object allocated with an alignment above 128 bytes and a
  size up to 8 KiB, to a larger size in the same size class, returned the object in place
  although its block had no room for the new size; writing to the end of it corrupted the
  allocator's next block. Such an object now moves.
