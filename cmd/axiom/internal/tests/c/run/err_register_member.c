/* expect-error: C3222 */
struct S { int m; };
int main(void) { register struct S s = { 1 }; int *p = &s.m; return *p; }
