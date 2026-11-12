## C

- C programs have atomics: `_Atomic` (the qualifier and `_Atomic(T)`) and
  `<stdatomic.h>`, on every target. Loads, stores, `++`, `--` and compound
  assignments of atomic objects are atomic and sequentially consistent;
  objects of 1, 2, 4 and 8 bytes are lock-free, larger ones use a lock.
  `__STDC_NO_ATOMICS__` is no longer defined. `__STDC_NO_THREADS__` still
  is: there is no `<threads.h>` yet.
- `_Thread_local` and `thread_local` objects have one instance per thread
  (they had static storage before), on every target: ELF TLS on Linux, the
  TLS directory on Windows, the runtime's thread-local block on macOS, and
  per-thread blocks on WebAssembly in programs with threads.
- Fixed: in a program linked statically on Linux, the main thread's copy of
  a thread-local whose initial value is an address held 0.
