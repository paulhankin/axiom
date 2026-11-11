/* expect-error: C3022 */
struct S { int a; } s;
int f(int);
int main(void) { return f(s); }
