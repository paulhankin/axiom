/* flags: --c-std=gnu17 */
/* expect-error: C3043 */
/* A nested function sees only the local labels (__label__) of its
   enclosing function: the others are not its labels. */
static int apply(int (*f)(int), int x) { return f(x); }
static int jump(int x) {
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
