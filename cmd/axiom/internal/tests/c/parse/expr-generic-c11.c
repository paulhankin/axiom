/* flags: --c-std=c11 */
/* A generic selection. */
int f(int x) {
    return _Generic(x, int: 1, const char *: 2, default: 3) +
           _Generic((x), long: x, int: x, int *: 0);
}
