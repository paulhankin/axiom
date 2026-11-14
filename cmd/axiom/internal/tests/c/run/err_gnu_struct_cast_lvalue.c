/* flags: --c-std=gnu17 */
/* expect-error: C3021 */
/* A GNU dialect's cast of a structure to its own type is a value, not an
   lvalue: neither it nor its members can be assigned. */
struct s { int a; };
int main(void) {
    struct s x = { 1 }, y = { 2 };
    ((struct s)x).a = y.a;
    return x.a;
}
