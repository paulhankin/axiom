/* flags: --c-std=c89 */
/* expect-error: C2107 */
int f(void) {
    int s = 0;
    for (int i = 0; i < 3; i++)
        s += i;
    return s;
}
