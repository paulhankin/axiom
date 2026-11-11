/* flags: --c-std=c17 */
/* expect-error: C2113 */
int f(void) {
    auto x = 1;
    return x;
}
