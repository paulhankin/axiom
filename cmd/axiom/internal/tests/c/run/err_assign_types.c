/* expect-error: C3022 */
struct S { int a; } s;
int main(void) { int x; x = s; return x; }
