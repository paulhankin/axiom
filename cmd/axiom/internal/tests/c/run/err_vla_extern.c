/* An identifier with a variably modified type has no linkage. */
/* expect-error: C3081 */
int f(int n) { extern int (*p)[n]; return 0; }
int main(void) { return f(1); }
