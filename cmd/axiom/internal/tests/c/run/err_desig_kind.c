/* expect-error: C3072 */
struct S { int a; } s = { [0] = 1 };
int main(void) { return 0; }
