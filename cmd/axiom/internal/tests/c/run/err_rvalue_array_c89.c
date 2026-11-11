/* flags: --c-std=c89 */
/* expect-error: C3020 */
/* in C89 an array that is not an lvalue is not converted to a pointer */
struct S { int a[2]; };
struct S f(void) { struct S s; s.a[0] = 1; s.a[1] = 2; return s; }
int main(void) { return f().a[1]; }
