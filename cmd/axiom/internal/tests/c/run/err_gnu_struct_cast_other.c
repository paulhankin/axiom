/* flags: --c-std=gnu17 */
/* expect-error: C3028 */
/* A GNU dialect casts a structure only to its own type: another structure
   type, even of the same members, stays a constraint violation. */
struct s { int a; };
struct t { int a; };
int main(void) {
    struct s x = { 1 };
    struct t y = (struct t)x;
    return y.a;
}
