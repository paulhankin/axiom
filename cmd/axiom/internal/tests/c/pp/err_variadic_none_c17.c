/* Before C23 the variable arguments cannot be left out. */
/* flags: --c-std=c17 */
/* expect-error: C1064 */
#define f(a, ...) a __VA_ARGS__
f(1)
