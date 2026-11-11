/* flags: --c-std=c23 */
#define f(a, ...) [a __VA_ARGS__]
f(1) f(1,) f(1, 2, 3)
