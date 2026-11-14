## C

- C programs have atomics: `_Atomic` (the qualifier and `_Atomic(T)`) and
  `<stdatomic.h>`, on every target. Loads, stores, `++`, `--` and compound assignments of
  atomic objects are atomic and sequentially consistent; objects of 1, 2, 4 and 8 bytes
  are lock-free, and those of 16 bytes aligned to 16 on AArch64 and x86-64
  (`cmpxchg16b`, inline with `--cpu=x86-64-v2` or later, else after asking the
  processor); others use a lock. `__STDC_NO_ATOMICS__` is no longer defined.
- The memory orders of `<stdatomic.h>`'s `_explicit` functions and of the GNU
  `__atomic` builtins are honoured: relaxed, acquire and release operations and fences
  are cheaper instructions on x86-64 and AArch64 (plain moves, `LDAR`/`STLR` and the
  acquire/release forms of the exclusive and LSE instructions, `DMB ISHLD`), and
  `atomic_compare_exchange_weak` is a single attempt on AArch64 without LSE. An order an
  operation does not take is the new warning `C4092`.
- The layout of `_Atomic` types is the platforms' compilers': on macOS, Windows and
  WebAssembly sizes up to 16 bytes (8 on WebAssembly) are padded to a power of two and
  aligned to it.
- When code from the platform's compilers is linked (`--link=`, `-l`, `--emit=obj`),
  atomic objects that are not lock-free go through the standard out-of-line interface
  (`__atomic_load`, `__atomic_compare_exchange` ...), which the program defines over its
  own locks, so that both sides' code takes the same lock; an executable linked with
  such code also defines the 16-byte sized forms (`__atomic_fetch_add_16` ...) over its
  own 16-byte compare-exchange (System V x86-64 and AAPCS64).
- Atomic objects of the decimal floating types, `_Float128`, `_FloatN`, imaginary and
  vector types work like any other object of their size.
- `_Thread_local` and `thread_local` objects have one instance per thread (they had
  static storage before), on every target: ELF TLS on Linux, the TLS directory on
  Windows, on macOS the runtime's thread-local block or the platform's thread-local
  variables (`--macos-tls`, below), and per-thread blocks on WebAssembly in programs with
  threads.
- Fixed: in a program linked statically on Linux, the main thread's copy of a
  thread-local whose initial value is an address held 0.
