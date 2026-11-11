/* flags: --c-std=c23 */
/* expect-error: C2001 */
int f(int x) { return (static int)x; }
