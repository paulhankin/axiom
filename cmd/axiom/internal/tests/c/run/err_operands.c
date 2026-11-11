/* expect-error: C3020 */
struct S { int a; } s;
int main(void) { return s + 1; }
