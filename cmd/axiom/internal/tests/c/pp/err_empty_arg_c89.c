/* Empty arguments are C99's. */
/* flags: --c-std=c89 */
/* expect-error: C1065 */
#define f(x) [x]
f()
