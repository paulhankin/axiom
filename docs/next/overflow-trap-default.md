## Language

- **Integer overflow traps by default.** `+`, `-`, `*`, unary `-`, `MIN / -1` and
  `@abs(MIN)` on integers, and `+=`, `-=`, `*=`, now trap with `T0002` when the result
  does not fit the type, in every build and at every optimization level (spec §7.6).
  Before, they wrapped. `+%`, `-%`, `*%` and `-%x` wrap, in every overflow mode, and are
  how a program asks for a wrap. Integer vector arithmetic and the `@atomic_*`
  arithmetic still wrap.

  To migrate: build and run your tests with `--overflow=report` (it wraps, and names
  each place that overflows once), then write each intended wrap (hashes, checksums,
  random number generators) with `%`, and fix the others. `--overflow=wrap` gives the
  old meaning for a whole build, for measuring.

  A reduction written with `+` (a sum, a dot product) no longer vectorizes, since the
  trap depends on the order of the additions; written with `+%` it does, as before.

  A C program (`axiom cc`, or a build whose first input is C) keeps C's arithmetic, and
  its Axiom runtime wraps as before; `--overflow` is still an error for it.
