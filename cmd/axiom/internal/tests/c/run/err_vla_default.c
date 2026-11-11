/* Nor a default label, in a block within the switch body. */
/* expect-error: C3084 */
int f(int n) {
    switch (n) {
    case 0: {
        int a[n];
    default:
        return 0;
    }
    }
    return 1;
}
int main(void) { return f(1); }
