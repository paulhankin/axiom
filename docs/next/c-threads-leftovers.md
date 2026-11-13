## C

- `thrd_sleep` returns -1 with the time left in `*remaining` when a signal handler ends
  it early (Linux, macOS); `tss_get` of a deleted key is null and `tss_set` of one
  `thrd_error`; `thrd_exit` on the main thread also waits for threads that Axiom code
  started. `thrd_create` and `thrd_join` cost about half the processor time they did
  (stacks are reused, and threads wake one another only when one waits).
