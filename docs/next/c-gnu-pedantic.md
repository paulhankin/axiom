## C

- With `--c-gnu-compat` (`axiom cc`'s default), the warning `C4145` for a later
  standard's keyword in an older GNU dialect (`_Atomic`, `_Bool` ... in gnu89, often
  from the system's headers) is shown only with the new `--c-pedantic`, which
  `axiom cc -pedantic`, `-Wpedantic` and `-pedantic-errors` give. Plain `--c-std=gnuNN`
  shows it as before.
