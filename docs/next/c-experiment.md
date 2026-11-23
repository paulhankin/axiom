## C

- The C front end (`axiom build file.c`, `axiom cc`) is an experiment ([`docs/c.md`](../c.md)):
  its tests run on every change of the suite, which counts them as a group of their
  own and exits with status 3 when only they failed, and a release of the language does
  not wait on them. These notes are its own section of the release notes.
- Its headers, C library and runtime moved from `std/c/` to `lib/c/` (`include/`,
  `lib/`, `rt/`), beside the standard library, where a release carries them too. A
  program that named them (`-I` to the headers, or a path into `std/c/rt`) names
  `lib/c/` now. `std/c`, the helpers for calling C from Axiom, stays.
