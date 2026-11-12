## C

- `<threads.h>`: threads (`thrd_create`, `thrd_join`, `thrd_detach`, `thrd_exit`,
  `thrd_current`, `thrd_sleep`, `thrd_yield`), mutexes (plain, timed, recursive),
  condition variables with `TIME_UTC` deadlines, thread-specific storage with its
  destructors, and `call_once` (also in C23's `<stdlib.h>`), on every target, on the
  Axiom runtime's threads. `__STDC_NO_THREADS__` is no longer defined. `thrd_exit` on
  the main thread lets the other threads finish, then ends the program with status 0;
  `exit` from any thread runs the `atexit` functions once. On WebAssembly without the
  atomics extension `thrd_create` returns `thrd_error`.
- The test-only functions `__axiom_c_thread_start` and `__axiom_c_thread_join` (and the
  start-up `std/internal/runtime/c_threads.ax`) are gone: use `<threads.h>`.
- The C library is safe to use from threads where the standard asks: `atexit`,
  `at_quick_exit` and `getenv` now may be called from several threads at once.

## Standard library

- `std/thread.exit()` ends the calling thread (one `spawn` started) at once, and
  `Thread.is_finished()` says whether a thread has ended. Hosts provide the new host
  function `ax_thread_exit` (the node host, and every native runtime).
