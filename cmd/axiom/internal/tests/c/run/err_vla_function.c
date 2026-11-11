/* A function cannot return a variably modified type (it has linkage). */
/* expect-error: C3081 */
int f(int n) { int (*g(void))[n]; return 0; }
int main(void) { return f(1); }
