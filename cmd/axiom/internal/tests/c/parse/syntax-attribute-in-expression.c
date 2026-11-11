/* flags: --c-std=c23 */
/* expect-error: C2005 */
int f(int *a) { return a[[0]]; }
