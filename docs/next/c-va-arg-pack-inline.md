## C

- An inline definition that forwards its caller's variable arguments
  (`__builtin_va_arg_pack ()`, `__builtin_va_arg_pack_len ()`) is inlined at each call,
  the pack replaced by the call's variable arguments. With `_FORTIFY_SOURCE` and the
  system's C library, the headers' wrappers of `sprintf`, `snprintf`, `printf`,
  `fprintf`, `dprintf`, `open`, `error`, `syslog` ... are therefore checked as with the
  GNU compilers: `sprintf` past the end of an array whose size the optimizer sees ends
  the program (`*** buffer overflow detected ***`), where before such calls went to the
  unchecked function. Code that `__builtin_constant_p` decides is never run is removed
  at every optimization level.
- New builtins: `__builtin_LINE`, `__builtin_FILE`, `__builtin_FUNCTION`,
  `__builtin___clear_cache` (on AArch64 the caches' lines cleaned and invalidated),
  `__builtin_speculation_safe_value` and `__builtin_bswap128`, which `__has_builtin`
  reports.
