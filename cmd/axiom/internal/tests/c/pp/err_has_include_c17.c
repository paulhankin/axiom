/* flags: --c-std=c17 */
/* expect-error: C1017 */
#if __has_include(<stddef.h>)
#endif
