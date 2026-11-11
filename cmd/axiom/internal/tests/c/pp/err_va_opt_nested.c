/* flags: --c-std=c23 */
/* expect-error: C1057 */
#define f(...) __VA_OPT__(__VA_OPT__())
