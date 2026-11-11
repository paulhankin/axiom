/* expect-error: C2022 */
int f(int a, int b) {
    a ? b : a = 1;
    return a;
}
