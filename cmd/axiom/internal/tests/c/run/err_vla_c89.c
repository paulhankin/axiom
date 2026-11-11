/* A variable length array is a C99 feature. */
/* flags: --c-std=c89 */
/* expect-error: C3080 */
int f(int n) { int a[n]; a[0] = 1; return a[0]; }
int main(void) { return f(1); }
