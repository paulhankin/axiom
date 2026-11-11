/* flags: --c-std=c11 */
/* Static assertions: at file scope, in a block, as a member (C11). */
_Static_assert(1, "file");
struct s {
    int a;
    _Static_assert(sizeof(int) > 1, "member");
};
int f(void) {
    _Static_assert(2 > 1, "block");
    return 0;
}
