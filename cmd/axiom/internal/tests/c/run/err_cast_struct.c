/* expect-error: C3028 */
struct S { int a; } s;
int main(void) { return (int)s; }
