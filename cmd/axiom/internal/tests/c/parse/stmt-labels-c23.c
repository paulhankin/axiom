/* flags: --c-std=c23 */
/* Labels before declarations and at the end of a block (C23). */
int f(int n) {
    switch (n) {
    case 1:
        int x = 1;
        n += x;
    default:
    }
a:
    int y = n;
    if (y > 9)
        goto a;
    {
    b:
    }
    return y;
}
