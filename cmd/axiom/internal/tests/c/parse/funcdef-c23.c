/* flags: --c-std=c23 */
/* C23 function definitions: unnamed parameters, '...' alone, () as (void). */
int f(int, int b) {
    return b;
}
int g(...);
int h(...) {
    return 0;
}
int k() {
    return 1;
}
