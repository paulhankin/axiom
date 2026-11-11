/* A directive inside the arguments of a macro is undefined behavior: an error. */
/* expect-error: C1043 */
#define f(x) x
f(1
#define Y 2
)
