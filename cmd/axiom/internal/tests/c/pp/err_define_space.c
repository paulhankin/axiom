/* C99: white space after an object-like macro's name. */
/* flags: --c-std=c99 */
/* expect-error: C1056 */
#define X+1
