/* expect-error: C3070 */
struct S { int a : 3; } s;
int *p = &s.a;
int main(void) { return 0; }
