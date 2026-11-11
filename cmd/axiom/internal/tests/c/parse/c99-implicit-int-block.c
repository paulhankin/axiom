/* flags: --c-std=c99 */
/* expect-error: C2113 */
int f(void) {
    register r = 1;
    return r;
}
