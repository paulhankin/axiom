/* expect-error: C2001 */
int f(int x) {
    if (x {
        return 1;
    }
    return 0;
}
