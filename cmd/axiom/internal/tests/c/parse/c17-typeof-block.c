/* flags: --c-std=c17 */
/* expect-error: C2100 */
int f(int x) {
    typeof(x) y = x;
    return y;
}
