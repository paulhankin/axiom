/* A typedef of a variably modified type cannot be declared again (C11 6.7p3). */
/* expect-error: C3002 */
int f(int n) { typedef int T[n]; typedef int T[n]; return 0; }
int main(void) { return f(1); }
