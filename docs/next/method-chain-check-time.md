## Compiler

- Fixed: a chain of method calls (`c.inc().inc()...`) took time exponential in its
  length to compile; a chain of 20 calls took seconds, one of 60 never finished.
