/* flags: --c-std=c89 */
/* expect-error: C2106 */
int f(void) {
    int a;
    a = 1;
    int b = a;
    a = b;
    int c = b;
    return c;
}
