## Compiler

- `--overflow=report`: a program built with it wraps on overflow, as by default, and the
  first time an operation that `--overflow=trap` checks (`+`, `-`, `*`, unary `-`,
  `MIN / -1`, `@abs`) overflows at a source location, it writes one line to its standard
  error, `file:line:col: integer overflow (T0002), wrapped: <the code>`, or appends it
  to the file that the environment variable `AXIOM_OVERFLOW_LOG` names. One run of a
  program or a test suite finds every site that overflows (spec §7.6). It works on
  every target; `axiom test` reports once per test, since each test runs in a process
  of its own.
- The test suite takes `--overflow=report` (and `--overflow=trap`) to build its Axiom
  programs and tests in that mode, and lists the sites reported at the end.

## Standard library

- `std/mem`: the count of bytes freed in each decay epoch saturates. On wasm32 a
  program that freed more than 4 GiB in one epoch (or in all, with no clock) wrapped
  it, and the allocator then kept less free memory than it should.
