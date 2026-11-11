/* flags: --c-std=c99 */
/* expect-error: C2100 */
int f(int x) { return _Generic(x, int: 1, default: 0); }
