/* [*] is not in a function definition's parameters. */
/* expect-error: C3085 */
int f(int n, int a[n][*]) { return 0; }
int main(void) { return 0; }
