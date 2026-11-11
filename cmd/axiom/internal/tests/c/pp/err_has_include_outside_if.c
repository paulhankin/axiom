/* flags: --c-std=c23 */
/* expect-error: C1049 */
int x = __has_include(<stddef.h>);
