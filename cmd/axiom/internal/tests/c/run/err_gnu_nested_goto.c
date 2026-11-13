/* flags: --c-std=gnu17 */
/* A jump from a nested function to a label of its enclosing function (a
   nonlocal goto) is not supported (C3901). */
/* expect-error: C3901 */
static int apply(int (*f)(int), int x) { return f(x); }
static int jump(int x) {
    __label__ out;
    int check(int y) {
        if (y < 0)
            goto out;
        return y * 2;
    }
    return apply(check, x);
out:
    return -100;
}
int main(void) { return jump(1) == 2 ? 0 : 1; }
