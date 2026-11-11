/* C23: a variable length array can be initialized with {} only. */
/* flags: --c-std=c23 */
/* expect-error: C3082 */
int f(int n) { int a[n] = { 0 }; return a[0]; }
int main(void) { return f(1); }
