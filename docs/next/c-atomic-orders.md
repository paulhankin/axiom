## C

- The memory orders of `<stdatomic.h>`'s `_explicit` functions and of the GNU
  `__atomic` builtins are honoured: relaxed, acquire and release operations and fences
  are cheaper instructions on x86-64 and AArch64 (plain moves, `LDAR`/`STLR` and the
  acquire/release forms of the exclusive and LSE instructions, `DMB ISHLD`), and
  `atomic_compare_exchange_weak` is a single attempt on AArch64 without LSE. An order an
  operation does not take is the new warning `C4092`.
- 16-byte atomic objects aligned to 16 are lock-free on AArch64 and x86-64
  (`cmpxchg16b`, inline with `--cpu=x86-64-v2` or later, else after asking the
  processor).
- The layout of `_Atomic` types on macOS, Windows and WebAssembly is now the platforms'
  compilers': sizes up to 16 bytes (8 on WebAssembly) are padded to a power of two and
  aligned to it. On Linux it is unchanged. Objects with such types compiled by an
  earlier version have other sizes.
- When code from the platform's compilers is linked (`--link=`, `-l`, `--emit=obj`),
  atomic objects that are not lock-free go through the standard out-of-line interface
  (`__atomic_load`, `__atomic_compare_exchange` ...), which the program defines over
  its own locks, so that both sides' code takes the same lock.
  That holds when our own linker links them too, and an executable linked with such
  code also defines the 16-byte sized forms (`__atomic_fetch_add_16` ...) over its own
  16-byte compare-exchange (System V x86-64 and AAPCS64).
- Atomic objects of the decimal floating types, `_Float128`, `_FloatN`, imaginary and
  vector types work like any other object of their size.
