/* expect-error: C3033 */
struct S { int n; int d[]; } s = { 1, { 2, 3 } };
int main(void) { return 0; }
