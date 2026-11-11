/* flags: --c-std=c95 */
/* expect-error: C2106 */
int f(void) {
    int a;
    a = 1;
    int b = a;
    return b;
}
