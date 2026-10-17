# 0013: The C compiler's native runtime does not build on macOS

- Type: feature
- Status: won't fix (not wanted)
- Area: runtime: `runtime/native_host.c`; compiler (C)

The C compiler (`cmd/axiom/bootstrap/`) emits code through LLVM for the host's default
target, so on a Mac it would produce Mach-O objects and link them with the
system C compiler. What stops it is `runtime/native_host.c`, the host ABI's
C implementation, which is Linux-specific:

- `<linux/futex.h>` and `syscall(SYS_futex, ...)` for `ax_wait32`/`ax_notify`
  and signal delivery: use `os_sync_wait_on_address`/`os_sync_wake_by_address_*`
  (macOS 14.4+) or `__ulock_wait`/`__ulock_wake` before that.
- `<sys/inotify.h>` for `ax_watch_*`: use kqueue `EVFILT_VNODE` (or FSEvents).
- `syscall(SYS_getrandom, ...)`: use `getentropy` (256 bytes at a time).
- `pipe2`, `accept4` and `MSG_NOSIGNAL`: Linux extensions; use `pipe` or
  `accept` plus `fcntl(FD_CLOEXEC)`, and `SO_NOSIGPIPE`.
- `ax_cpu_count`, `ax_term_size` and similar: check each against Darwin.

`cmd/axiom/bootstrap/src` itself may assume ELF in places (section names for the trap
site records, `axiom.ss` thread-local model, symbol names without the
leading underscore) and its Makefile assumes a Linux LLVM install.

This is the cheapest route to native Mac binaries: an LLVM-built compiler on
a Mac host building Mac programs. It does not help cross-compiling from
Linux, and the self-hosted compiler (0011) is still the long-term route.
