/* flags: --c-std=c99 */
/* expect-error: C3009 */
struct S { int n; char d[0]; };
int main(void) { return 0; }
