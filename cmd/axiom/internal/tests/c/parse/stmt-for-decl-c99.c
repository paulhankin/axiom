/* flags: --c-std=c99 */
/* A declaration in for (C99), in a scope of its own. */
typedef int T;
int f(int n) {
    int s = 0;
    for (int T = 0, j = 1; T < n; T++, j++)
        s += T * j;
    for (T i = 0; i < n; i++)
        s += i;
    return s;
}
