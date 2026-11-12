/* flags: --c-std=gnu99 -Ignu_inc1 -Ignu_inc2 */
/* The GNU dialects' preprocessor: named variable arguments, `, ##
   __VA_ARGS__`, variadic macros invoked without their variable arguments,
   __COUNTER__, __INCLUDE_LEVEL__, __BASE_FILE__, #include_next and
   __has_include_next, #warning before C23, #ident, '\e' and binary
   constants, __has_attribute and __has_builtin of what the dialect has. */
/* expect-warning: C4103 */
#define LOG(fmt, args...) printf(fmt, ## args)
#define ERR(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define TWO(a, ...) [a | __VA_ARGS__]
LOG("x") LOG("y", 1, 2)
ERR("z") ERR("w", q)
TWO(1) TWO(1, 2)
__COUNTER__ __COUNTER__ __COUNTER__
__INCLUDE_LEVEL__ __BASE_FILE__
#include <gnu_next.h>
#warning this is a warning
#ident "version 1"
#if __has_attribute(packed) && __has_attribute(gnu::aligned) && !__has_attribute(nosuch) && __has_attribute(deprecated)
attrs
#endif
#if __has_builtin(__builtin_expect) && __has_builtin(__sync_fetch_and_add) && !__has_builtin(__builtin_nosuch)
builtins
#endif
#if defined(__AXIOM_GNU__) && !defined(__GNUC__) && __ATOMIC_SEQ_CST == 5
dialect
#endif
#if 0b11 == 3
binary
#endif
