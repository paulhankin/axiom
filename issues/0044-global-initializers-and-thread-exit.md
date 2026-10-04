# 0044: Restricted global initializers; no thread-exit cleanup for thread-locals

- Type: feature
- Status: open
- Area: language, std/thread

Found porting the benchmark's evaluator (`axiom/src/eval/`):

1. Global and `threadlocal` initializers cannot call functions or hold an empty slice
   (only `""` works for `[]u8`), so a table allocated at first use has to be kept as a
   `usize` address that is 0 until then, and per-thread scratch is created lazily by hand
   on every entry point.
2. Thread-locals have no destructors and threads no exit hooks, so heap scratch owned by a
   thread-local (Rust's `thread_local!` with `Drop`) must be freed explicitly by each
   worker before it returns (`prune.free_thread_scratch()`), or it leaks.

Proposal: allow empty slices (`&[]`, a null pointer with length 0) and other constant
expressions of any type in initializers; and either `threadlocal` destructors (a `deinit`
called at thread exit, by `std/thread` for threads it spawns) or a `thread.on_exit(fn)`
hook.
