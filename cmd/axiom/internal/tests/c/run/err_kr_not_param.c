/* flags: --c-std=c89 */
/* expect-error: C3010 */
int f(a) int a; int b; { return a; }
int main(void) { return 0; }
