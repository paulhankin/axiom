## WebAssembly

- Threaded programs no longer trap now and then with "memory access out of bounds"
  when a thread touches memory that another thread has just added by growing the
  memory. Engines may keep each thread's view of a shared memory's size and update it
  late; atomic and bulk-memory accesses are checked against that view. The compiler
  now keeps every thread's view current: a growth records the memory's size in a
  word, and an atomic operation that finds the word beyond the thread's view refreshes
  it (spec §12.8.5, "Memory growth"). This costs one load and one compare per atomic
  operation.
- `exit`, `_Exit`, `quick_exit` and `abort` on a thread other than the main one (and
  `std/os.exit` on any thread) end a WebAssembly program at once. Before, they waited
  until the main thread next waited in `<threads.h>`, and Axiom's `os.exit` ended only
  its own thread. The host interface now says that `ax_exit` ends the program from any
  thread. The node host (`std/internal/runtime/host.mjs`) runs a threaded program's
  main thread on a worker too, under a supervisor on node's main thread. The threads
  now share the environment, and any of them may change the working directory.
- The node host runs wasm64 programs that use threads or the host's system functions
  (std/io, the C library). The suite's `[c]`, `[c-mix]` and `[c-lib]` sections run on
  wasm64 with `AXIOM_C_TARGET=wasm64`, instead of being built freestanding or only
  built.
- Local time on WebAssembly no longer needs the host's time zone files: a new host
  function, `ax_tz_local`, gives the host's local zone as a TZif file. The node host
  builds it from what the JavaScript engine knows of the zone. `std/time/tz`'s
  `Zone.local` (and so C's `localtime`, `mktime` and `strftime`'s `%z` and `%Z`) uses
  it when `$TZ` and `/etc/localtime` give no zone; `Zone.from_host` asks for it
  directly.
- The node host can confine a program's files to a directory (`--root=dir`, which the
  program sees as `/`) or keep them in memory (`--files=memory`, with
  `std/internal/runtime/memfs.mjs`, which a browser's host can use as it is). The C
  library's file functions (`fopen` and the rest, `remove`, `rename`, `tmpfile`) work
  in all three modes.
- C's `setjmp` and `longjmp` on WebAssembly, with `--features=+exception-handling` (the
  standard form of the extension: `try_table`, `throw`, a tag). Without it `setjmp` is
  still C3300, whose message now names the flag. `<setjmp.h>`'s tests run on wasm32
  and wasm64.
- With `--features=+tail-call`, sibling calls (at `-O2`) are `return_call` and
  `return_call_indirect`, so mutual recursion in tail position no longer grows the
  stack. Axiom functions qualify too: their stack check no longer prevents it.
- With bulk-memory (on by default), the C library's `memcpy`, `memmove` and `memset`
  are `memory.copy` and `memory.fill`. Under node they ran 1.3 to 60 times as fast as
  the C loops they replace. `__AXIOM_WASM__`, `__AXIOM_WASM_BULK_MEMORY__` and
  `__AXIOM_WASM_EXCEPTION_HANDLING__` are predefined where they apply.
- `-g` on WebAssembly now describes types and variables as well as lines. Each object
  carries its own DWARF, which the linker merges. At `-O0` variables are in their
  frame slots, from the frame's local (`DW_OP_WASM_location`); Axiom locals stay in
  memory at `-O0` there too, as natively. A stack overflow in such a frame is now caught
  by the frame's own stack check, which reports the function's line. Optimized code's
  variables have no location on WebAssembly yet.
