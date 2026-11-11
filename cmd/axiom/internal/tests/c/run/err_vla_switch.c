/* A case label cannot be in the scope of a VLA declared in its switch body. */
/* expect-error: C3084 */
int f(int n) {
    switch (n) {
        int a[n];
    case 1:
        a[0] = 1;
        return a[0];
    }
    return 0;
}
int main(void) { return f(1); }
