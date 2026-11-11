/* flags: --c-std=c23 */
/* Storage-class specifiers in compound literals (C23). */
int *f(void) {
    return (static int[]){1, 2};
}
int g(void) {
    return (register int){3} + (constexpr int){4};
}
