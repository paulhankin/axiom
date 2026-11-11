/* flags: --c-std=c89 */
/* expect-error: C3002 */
int f(a, a) int a; { return a; }
int main(void) { return 0; }
