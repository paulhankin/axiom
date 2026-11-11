/* expect-error: C3022 */
struct S { int a; } s;
int f(void) { return s; }
int main(void) { return f(); }
