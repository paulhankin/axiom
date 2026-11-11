/* expect-error: C2027 */
int f(int x) {
    return ++(int)x;
}
