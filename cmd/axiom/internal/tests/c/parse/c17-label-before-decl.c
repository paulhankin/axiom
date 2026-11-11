/* flags: --c-std=c17 */
/* expect-error: C2138 */
int f(void) {
l:
    int x = 1;
    return x;
}
