/* flags: --c-std=c17 */
/* expect-error: C2140 */
int *f(void) { return (static int[]){1}; }
