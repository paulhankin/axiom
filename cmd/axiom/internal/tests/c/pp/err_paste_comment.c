/* // is not a token. */
/* flags: --c-std=c99 */
/* expect-error: C1067 */
#define cat(a, b) a ## b
cat(/, /)
