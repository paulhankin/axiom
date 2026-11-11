/* A variable length array cannot be initialized (before C23). */
/* expect-error: C3082 */
int f(int n) { int a[n] = { 1 }; return a[0]; }
int main(void) { return f(1); }
