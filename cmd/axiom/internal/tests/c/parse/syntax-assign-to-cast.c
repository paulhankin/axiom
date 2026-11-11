/* expect-error: C2022 */
int f(int a) {
    (long)a = 1;
    return a;
}
