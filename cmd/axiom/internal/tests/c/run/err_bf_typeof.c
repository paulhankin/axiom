/* flags: --c-std=c23 */
/* expect-error: C3070 */
struct S { int a : 3; } s;
typeof(s.a) x;
int main(void) { return 0; }
