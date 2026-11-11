/* # in a function-like macro must be followed by a parameter. */
/* expect-error: C1059 */
#define f(x) # y
