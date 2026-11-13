## C front end

- `--macos-tls=fast|platform` chooses macOS's thread-locals (C's `_Thread_local` and
  Axiom's `threadlocal`). `fast`, the default, is the runtime's per-thread block as before,
  and its output is unchanged. `platform` uses the platform's thread-local variables: a
  descriptor in `__thread_vars` for each, its initial value in `__thread_data`/
  `__thread_bss`, code that calls the descriptor's thunk (arm64 and x86-64), dyld making
  each thread's block, the debugger reading the descriptors. Such thread-locals link with
  the platform's linker and the platform's compilers' objects and libraries both ways,
  and every thread has them. `platform` has never been executed (built and checked here
  only); it assumes the thunk changes nothing but x0, x16, x17, lr and the flags (arm64)
  or rax, rdi and the flags (x86-64), costs about 14 instructions and a call an access,
  and a frameless function cannot be unwound past while the thunk runs. The default may
  change once it has run on a Mac.
- `--emit=obj`, `--emit=lib` and `--emit=shared` for macOS imply `platform`: objects for
  the platform's toolchain have its thread-locals, and dynamic libraries may now have and
  export thread-locals.
- The linker makes the descriptor references of the image's own variables direct (the
  `ldr` of `@TLVPPAGEOFF` an `add`, x86-64's `movq` a `leaq`). Code of one scheme reaching
  the other's variable is a link error, as is an undefined thread-local reached by its
  descriptor that no library given to the link can define (it was bound to libSystem).
- x86-64 assembly on macOS: `sym@TLVP(%rip)` (`X86_64_RELOC_TLV`).
