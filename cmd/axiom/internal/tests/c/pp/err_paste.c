/* ## must make a valid preprocessing token. */
/* expect-error: C1067 */
#define cat(a, b) a ## b
cat(+, -)
