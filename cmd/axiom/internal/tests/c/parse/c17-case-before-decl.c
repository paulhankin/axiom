/* flags: --c-std=c17 */
/* expect-error: C2138 */
int f(int n) {
    switch (n) {
    case 1:
        int x = 2;
        return x;
    }
    return 0;
}
