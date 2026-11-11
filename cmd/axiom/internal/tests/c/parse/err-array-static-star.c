/* flags: --c-std=c99 */
/* expect-error: C2001 */
/* 6.7.5.2: 'static' in an array declarator is followed by a size, not '*'. */
int f(int n, int a[static *]);
