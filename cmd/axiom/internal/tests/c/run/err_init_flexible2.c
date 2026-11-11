/* expect-error: C3033 */
struct S { int n; int d[]; } s = { .d = { 2 } };
int main(void) { return 0; }
