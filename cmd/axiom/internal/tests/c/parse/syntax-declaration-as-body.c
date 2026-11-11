/* expect-error: C2011 */
int f(int x) {
    if (x)
        int y = 1;
    return 0;
}
