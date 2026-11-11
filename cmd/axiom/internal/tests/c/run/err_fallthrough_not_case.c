/* C23: a fallthrough declaration must be followed by a case or default label. */
/* flags: --c-std=c23 */
/* expect-error: C3077 */
int f(int x) {
    switch (x) {
    case 1:
        x++;
        [[fallthrough]];
        x++;
    case 2:
        return x;
    }
    return 0;
}
int main(void) { return f(1); }
