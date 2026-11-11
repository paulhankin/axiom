/* expect-error: C3070 */
struct S { int a : 3; } s;
unsigned long n = sizeof s.a;
int main(void) { return 0; }
