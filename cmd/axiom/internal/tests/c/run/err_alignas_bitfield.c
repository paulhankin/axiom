/* expect-error: C3071 */
struct S { _Alignas(8) int a : 3; };
int main(void) { return 0; }
