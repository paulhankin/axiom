/* flags: --c-std=c23 */
/* expect-error: C2142 */
int f(a, b) int a; int b; { return a + b; }
