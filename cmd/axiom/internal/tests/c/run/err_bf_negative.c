/* expect-error: C3070 */
struct S { int a; unsigned : -1; };
int main(void) { return 0; }
