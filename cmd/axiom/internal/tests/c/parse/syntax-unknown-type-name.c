/* expect-error: C2025 */
/* expect-error: C2025 */
foo x;
int f(void) {
    bar y = 1;
    y = 2;
    bar *z = &y;
    return *z;
}
