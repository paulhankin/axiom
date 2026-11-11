/* flags: --c-std=c23 */
/* expect-error: C1058 */
#define f(...) __VA_OPT__(## x)
