/* expect-error: C3072 */
struct S { int a; } s = { .a.b = 1 };
int main(void) { return 0; }
