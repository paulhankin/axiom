/* expect-error: C3070 */
struct S { int a : 0; };
int main(void) { return 0; }
