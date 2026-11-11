/* flags: --c-std=c99 */
/* expect-error: C3001 */
/* A K&R parameter is in scope only after its own declarator: not in the
   size of an array declared before it. */
int f(a, n) int a[n]; int n; { return n; }
int main(void) { return 0; }
