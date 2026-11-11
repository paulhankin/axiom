/* flags: --c-std=c23 */
/* expect-error: C1023 */
#define f(x) __VA_OPT__(x)
