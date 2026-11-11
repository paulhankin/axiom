/* flags: --c-std=c23 */
/* expect-error: C2142 */
/* C23 has no identifier lists */
int f(a) int a; { return a; }
int main(void) { return 0; }
