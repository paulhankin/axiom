/* expect-error: C3025 */
struct S { int a; } s;
int main(void) { return s.b; }
