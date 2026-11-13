## C

- A C source given with `--link=file.c` for a macOS or Windows target is compiled by the
  compiler itself (in a process of its own, with the build's target and C options) into an
  object in the target's format, instead of by the system's C compiler, which compiles
  only for Linux here; for Linux targets it is still the system's.
