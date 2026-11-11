/* A function-like macro's name without ( is not an invocation; ( may be on a later line. */
#define f(x) [x]
#define g f
#define LP (
f g f LP 1) f;
f
(2)
g
 (3)
f
#define X 4
(X)
int f = 0, (f)(5);
