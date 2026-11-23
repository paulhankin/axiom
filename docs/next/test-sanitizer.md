## Tools

- `axiom test` builds the tests with the address sanitizer (`--sanitize=address`, spec
  §16.8) unless given `--sanitize=none`. A test that uses memory after freeing it (a
  slice kept across a `Vec` push that moved the buffer, say) now fails with `T0012`, and
  a test that leaves allocations unfreed fails with `leak`; to update, free what the
  test allocates (`defer`), or keep memory on purpose inside `mem.leaks_ignored`. Tests
  take longer (the standard library's: 1.8 times as long natively, 1.6 on WebAssembly),
  so a test's default time limit is 30 s with the sanitizer. The default applies to
  Axiom programs on every target but `wasm64`; for `wasm64`, a C program, or an Axiom
  program with C units, `axiom test` builds without the sanitizer, as before.
- The sanitizer's quarantine of freed memory is 16 MiB in a test (64 MiB elsewhere),
  since tests run in parallel, and the sanitizer is faster: it sets and checks shadow
  memory a page at a time, and remembers which allocation sites are `std/mem`'s.
- A new warning, `W0711`, on by default and turned off with `--no-warn=W0711`, reports a
  local slice or pointer used after the memory it points into was freed or may have
  moved, in the two plain shapes one function shows: after it was passed to `mem.free`
  (or `mem.destroy`), or after a `*mut Self` method was called on the container it came
  from through a `*Self` method (`let xs = v.items(); v.push(x)!; xs[0]`). It is a
  heuristic, with false positives.

## Language

- The specification (§1, §12.1), the README and the tour state the memory-safety claim
  as it is: every operation is checked or defined, except two rules a program keeps
  itself. A raw pointer stays inside its object, and memory is not used after it is
  freed, through a pointer or a slice. The sanitizer catches breaks of the second.
