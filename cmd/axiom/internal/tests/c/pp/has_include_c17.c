/* flags: --c-std=c17 */
/* __has_include, __has_attribute and __has_builtin are the compiler's in
   every mode (reserved identifiers); __has_c_attribute stays C23's. */
#if __has_include(<stddef.h>) && !__has_include("nope.h")
ok1
#endif
#if !__has_attribute(packed) && !__has_attribute(deprecated)
ok2
#endif
#if __has_builtin(__builtin_offsetof) && !__has_builtin(__builtin_expect)
ok3
#endif
#ifdef __has_include
ok4
#endif
