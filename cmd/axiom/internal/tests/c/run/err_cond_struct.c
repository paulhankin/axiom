/* expect-error: C3030 */
struct S { int a; } s;
int main(void) { if (s) return 1; return 0; }
