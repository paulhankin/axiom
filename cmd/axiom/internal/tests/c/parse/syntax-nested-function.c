/* expect-error: C2015 */
int f(void) {
    int g(void) { return 1; }
    return 0;
}
