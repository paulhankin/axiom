/* flags: --c-std=gnu17 */
/* expect-error: C3028 */
/* expect-error: C3026 */
/* A cast to a union of a value whose type is no member's is an error (the
   conversion is not looked for: 1.5 is no int); the cast is not an lvalue. */
union u { int i; char *p; };
int main(void) {
    union u a = (union u)1.5;
    union u *p = &(union u)3;
    return a.i + p->i;
}
